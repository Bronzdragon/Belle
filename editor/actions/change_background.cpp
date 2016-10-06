/* Copyright (C) 2012-2014 Carlos Pais
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "change_background.h"
#include "scene_manager.h"
#include "utils.h"
#include "assetmanager.h"

#include <QFileInfo>
#include <QtDebug>

ChangeBackground::ChangeBackground(QObject *parent) :
    Action(parent)
{
    init();
}

ChangeBackground::ChangeBackground(const QVariantMap & data, QObject *parent):
    Action(data, parent)
{
    init();
    if (data.contains("backgroundImage") && data.value("backgroundImage").type() == QVariant::String)
        setBackgroundImage(data.value("backgroundImage").toString());

    if (data.contains("backgroundColor") && data.value("backgroundColor").type() == QVariant::List)
        setBackgroundColor(Utils::listToColor(data.value("backgroundColor").toList()));

}

ChangeBackground::~ChangeBackground()
{
    AssetManager::instance()->releaseAsset(mBackgroundImage);
}

void ChangeBackground::init()
{
    setType(GameObjectMetaType::ChangeBackground);

    mBackgroundImage = 0;
}

void ChangeBackground::setBackgroundImage(const QString & background)
{ 
    ImageFile* image = dynamic_cast<ImageFile*>(AssetManager::instance()->loadAsset(background, Asset::Image));
    if (mBackgroundImage == image) {
        AssetManager::instance()->releaseAsset(image);
        return;
    }

    Scene* scene = this->scene();
    if (scene && scene->temporaryBackgroundImage() == mBackgroundImage)
        scene->setTemporaryBackgroundImage(0);

    AssetManager::instance()->releaseAsset(mBackgroundImage);
    mBackgroundImage = image;

    QFileInfo info(background);
    setDisplayText(info.fileName());
    emit dataChanged();

    if (scene && scene->temporaryBackgroundImage() != mBackgroundImage)
        focusIn();


}

ImageFile* ChangeBackground::background() const
{
    return mBackgroundImage;
}

QString ChangeBackground::backgroundPath()
{
    if (mBackgroundImage)
        return mBackgroundImage->path();
    return "";
}

void ChangeBackground::setBackgroundColor(const QColor& color)
{
    mBackgroundColor = color;
    if (color.isValid())
        setDisplayText(color.name());
    emit dataChanged();

    Scene* scene = this->scene();
    if (scene && scene->temporaryBackgroundColor() != color)
        focusIn();
}

QColor ChangeBackground::backgroundColor()
{
    return mBackgroundColor;
}

void ChangeBackground::focusIn()
{
    Action::focusIn();
    Scene *scene = this->scene();

    if (scene) {
        scene->setTemporaryBackgroundColor(mBackgroundColor);
        scene->setTemporaryBackgroundImage(mBackgroundImage);
    }
}

void ChangeBackground::focusOut()
{
    Action::focusOut();
    Scene* scene = this->scene();

    if (scene) {
        scene->setTemporaryBackgroundImage(0);
        scene->setTemporaryBackgroundColor(QColor());
    }
}

QVariantMap ChangeBackground::toJsonObject(bool internal) const
{
    QVariantMap data = Action::toJsonObject(internal);
    QString path = mBackgroundImage ? mBackgroundImage->path() : "";

    if (! path.isEmpty())
        data.insert("backgroundImage", QFileInfo(path).fileName());
    if (mBackgroundColor.isValid())
        data.insert("backgroundColor", Utils::colorToList(mBackgroundColor));

    return data;
}
