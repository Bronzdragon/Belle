#include "assetmanager.h"

#include <QFile>
#include <QFontDatabase>

#include "utils.h"
#include "fontfile.h"
#include "json.h"

static AssetManager* mInstance = new AssetManager();

AssetManager::AssetManager()
{
    mTypeToPath.insert(Asset::Image, "");
    mTypeToPath.insert(Asset::Audio, "");
    mTypeToPath.insert(Asset::Font, "");
    mTypeToPath.insert(Asset::Unknown, "");

    mImageFormats << "bmp" << "gif" << "ico" << "jpeg" << "jpg" << "mng" << "png" << "svg" << "svgz" << "tif" << "tiff" << "webp" << "xbm" << "xpm";
    mAudioFormats << "mp3" << "aac" << "ogg" << "oga" << "webm" << "wav" << "m4a" << "mp4";
    mVideoFormats << "mp4" << "m4v" << "webm" << "ogv" << "ogg";
    mFontFormats << "ttf" << "otf" << "eot" << "woff";
}

AssetManager* AssetManager::instance()
{
    return mInstance;
}

void AssetManager::setLoadPath(const QString & path)
{
    mLoadPath = path;
}

QString AssetManager::absoluteFilePath(const QString & name, Asset::Type type)
{
    if (! mLoadPath.isEmpty()) {
         QDir dir(mLoadPath);
         if (! mTypeToPath[type].isEmpty() && dir.exists(mTypeToPath[type]))
             dir.cd(mTypeToPath[type]);
         if (dir.exists(name))
             return dir.absoluteFilePath(name);
    }

    return name;
}

Asset* AssetManager::asset(const QString & path, Asset::Type type) const
{
    if (type == Asset::Unknown)
        type = guessType(path);

    bool absolute = QFileInfo(path).isAbsolute();
    QList<Asset*> assets = mAssets.keys();
    for(int i=0; i < assets.size(); i++) {
        if (assets[i]->type() != type)
            continue;

        if (absolute && assets[i]->path() == path || (! absolute) && assets[i]->name() == path)
            return assets[i];
    }
    return 0;
}

QList<Asset*> AssetManager::assets() const
{
    return mAssets.keys();
}

QList<Asset*> AssetManager::assets(Asset::Type type) const
{
    QList<Asset*> assets = mAssets.keys();
    for(int i=assets.size()-1; i >= 0; i--)
        if (assets[i]->type() != type)
            assets.removeAt(i);
    return assets;
}

Asset* AssetManager::loadAsset(QString path, Asset::Type type)
{
    path = absoluteFilePath(path);
    Asset* asset = this->asset(path, type);
    if (asset) {
        int count = mAssets.value(asset, 0);
        mAssets.insert(asset, ++count);
        return asset;
    }

    asset = _loadAsset(path, type);
    if (asset)
        mAssets.insert(asset, 1);
    return asset;
}

Asset* AssetManager::_loadAsset(const QString& path, Asset::Type type) const
{
    if (! QFile::exists(path))
        return 0;

    Asset* asset = 0;
    if (type == Asset::Image)
        asset = ImageFile::create(path);
    else if (type == Asset::Font)
        asset = new FontFile(path);
    else
        asset = new Asset(path, type);

    QString destPath = mTypeToPath.value(type, "");
    if (asset && ! isNameUnique(asset->name(), destPath))
        asset->setName(uniqueName(asset->name(), destPath));
    return asset;
}

void AssetManager::releaseAsset(Asset* asset)
{
    if (! asset)
        return;

    int count = mAssets.value(asset, 0);
    if (count) {
        --count;
        if (count <= 0) {
            mAssets.remove(asset);
            delete asset;
        }
        else
            mAssets.insert(asset, count);
    }
}

void AssetManager::removeAssets()
{
    QHashIterator<Asset*, int> it(mAssets);
    while(it.hasNext()) {
        it.next();
        if (it.key())
            delete it.key();
    }
    mAssets.clear();
}

ImageFile* AssetManager::image(const QString & name)
{
   return dynamic_cast<ImageFile*>(asset(name));
}

bool AssetManager::isNameUnique(const QString& name, const QString& destPath) const
{
    QList<Asset*> assets = mAssets.keys();
    for(int i=0; i < assets.size(); i++)
        if (assets[i]->name() == name && mTypeToPath.value(assets[i]->type(), "") == destPath)
            return false;

    return true;
}


QString AssetManager::uniqueName(QString name, const QString& destPath) const
{
    if (name.isEmpty() || name.isNull())
        name = "asset";

    while(! isNameUnique(name, destPath))
        name = Utils::incrementLastNumber(name);

    return name;
}

Asset::Type AssetManager::guessType(const QString & name) const
{
    QFileInfo info(name);
    QString ext = info.suffix();
    if (mImageFormats.contains(ext, Qt::CaseInsensitive))
        return Asset::Image;
    if (mAudioFormats.contains(ext, Qt::CaseInsensitive))
        return Asset::Audio;
    if (mVideoFormats.contains(ext, Qt::CaseInsensitive))
        return Asset::Video;
    if (mFontFormats.contains(ext, Qt::CaseInsensitive))
        return Asset::Font;
    return Asset::Unknown;
}

QVariantMap AssetManager::readAssetsFile(const QString& filepath)
{
    QFile file(filepath);
    if (! file.open(QFile::ReadOnly | QFile::Text))
        return QVariantMap();

    QString contents = file.readAll();
    //remove start "game.assets ="
    int i = 0;
    for(i=0; i < contents.size() && contents[i] != '{'; i++);
    contents = contents.mid(i);

    bool ok;
    QVariant data = QtJson::Json::parse(contents, ok);
    if (! ok)
        return QVariantMap();

    if (data.type() != QVariant::Map)
        return QVariantMap();

    return data.toMap();
}

void AssetManager::load(const QDir & dir)
{
    QVariantMap data = readAssetsFile(dir.absoluteFilePath(ASSETS_FILE));
    if (data.isEmpty())
        return;

    QVariantMap subdirs = data.value("subdirs").toMap();
    mTypeToPath[Asset::Image] = subdirs.value("images", "").toString();
    mTypeToPath[Asset::Audio] = subdirs.value("sounds", "").toString();
    mTypeToPath[Asset::Font] = subdirs.value("fonts", "").toString();

    QVariantMap item;

    QVariantList imagesData = data.value("images").toList();
    for(int i=0; i < imagesData.size(); i++) {
        item = imagesData[i].toMap();
        loadAsset(item.value("name", "").toString(), Asset::Image);
    }

    QVariantList audioData = data.value("sounds").toList();
    for(int i=0; i < audioData.size(); i++) {
        item = audioData[i].toMap();
        loadAsset(item.value("name", "").toString(), Asset::Audio);
    }

    QVariantList fontsData = data.value("fonts").toList();
    for(int i=0; i < fontsData.size(); i++) {
        item = fontsData[i].toMap();
        loadAsset(item.value("name", "").toString(), Asset::Font);
    }

    //reset reference counts
    QList<Asset*> assets = mAssets.keys();
    for(int i=0; i < assets.size(); i++)
        mAssets[assets[i]] = 0;
}

void AssetManager::save(const QDir & dir)
{
    QVariantMap data;
    QFile file(dir.absoluteFilePath(ASSETS_FILE));
    if (! file.open(QFile::WriteOnly | QFile::Text))
        return;

    QVariantMap subdirs;
    subdirs.insert("images", mTypeToPath.value(Asset::Image));
    subdirs.insert("sounds", mTypeToPath.value(Asset::Audio));
    subdirs.insert("fonts", mTypeToPath.value(Asset::Font));
    data.insert("subdirs", subdirs);

    QList<Asset*> images = this->assets(Asset::Image);
    QVariantList imagesData;
    for(int i=0; i < images.size(); i++) {
        imagesData.append(images[i]->toJsonObject());
        images[i]->save(dir);
    }

    QList<Asset*> sounds = this->assets(Asset::Audio);
    QVariantList soundsData;
    for(int i=0; i < sounds.size(); i++) {
        soundsData.append(sounds[i]->toJsonObject());
        sounds[i]->save(dir);
    }

    QList<Asset*> fonts = this->assets(Asset::Font);
    QVariantList fontsData;
    for(int i=0; i < fonts.size(); i++) {
        fontsData.append(fonts[i]->toJsonObject());
        fonts[i]->save(dir);
    }

    data.insert("images", imagesData);
    data.insert("sounds", soundsData);
    data.insert("fonts", fontsData);

    file.write("game.assets = ");
    file.write(QtJson::Json::serialize(data));
    file.close();

    saveFontFaces(fonts, dir);
}

void AssetManager::saveFontFaces(const QList<Asset*>& fonts, const QDir& dir)
{
    if (fonts.isEmpty())
        return;

    QString fontfaces(FONTFACES_FILE);
    QFile file(dir.absoluteFilePath(fontfaces));
    if (! file.open(QFile::WriteOnly | QFile::Text))
        return;

    FontFile* font = 0;

    for(int i=0; i < fonts.size(); i++) {
        font = dynamic_cast<FontFile*>(fonts[i]);
        if (! font) continue;
        QString basename = QFileInfo(font->path()).baseName();

        //determine font's family name.
        QStringList families = QFontDatabase::applicationFontFamilies(font->id());
        for(int j=0; j < families.size(); j++) {
            //write css for font
            file.write(Utils::fontFace(basename, families[j]).toAscii() + "\n");
        }
    }

    file.close();
}

QList<int> AssetManager::fontsIds()
{
    QList<int> ids;
    QList<Asset*> fontfiles = this->assets(Asset::Font);
    for(int i=0; i < fontfiles.size(); i++) {
        FontFile* font = dynamic_cast<FontFile*>(fontfiles[i]);
        ids.append(font->id());
    }

    return ids;
}
