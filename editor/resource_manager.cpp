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

#include "resource_manager.h"

#include <QDebug>
#include <QMovie>

#include "utils.h"
#include "image.h"
#include "character.h"
#include "dialoguebox.h"
#include "textbox.h"
#include "button.h"
#include "menu.h"
#include "animatedimage.h"
#include "gameobjectmanager.h"
#include "gameobjectfactory.h"

static ResourceManager* mInstance = new ResourceManager();

ResourceManager::ResourceManager(QObject *parent) :
    GameObjectManager(parent)
{
}

void ResourceManager::add(GameObject * obj)
{
    if (obj && obj->parent() != this)
        obj->setParent(this);

    GameObjectManager::add(obj);
}

void ResourceManager::add(const QVariantMap& data)
{
    add(createGameObject(data, this));
}

GameObject* ResourceManager::createGameObject(const QVariantMap& info, QObject* parent)
{
    GameObject* _resource = 0;
    QVariantMap data = info;

    fillWithResourceData(data);

    if (data.contains("resource") && data.value("resource").type() == QVariant::String)
        _resource = this->object(data.value("resource").toString());

    GameObject* object = GameObjectFactory::createGameObject(data, parent);
    if (object && _resource)
        object->setResource(_resource);

    return object;
}

Object* ResourceManager::createObject(const QVariantMap& info, QObject* parent)
{
    return qobject_cast<Object*>(createGameObject(info, parent));
}

void ResourceManager::fillWithResourceData(QVariantMap& data)
{
    if (! data.contains("resource") || data.value("resource").type() != QVariant::String)
        return;

    QString name = data.value("resource").toString();

    GameObject* resource = object(name);
    if (! resource)
        return;

    QVariantMap resourceData = resource->toJsonObject();
    QStringList keys = resourceData.keys();
    foreach(const QString& key, keys) {
        if (! data.contains(key))
            data.insert(key, resourceData.value(key));
    }
}

void  ResourceManager::destroy()
{
    mInstance->clear(true);
    mInstance->deleteLater();
}

ResourceManager* ResourceManager::instance()
{
    return mInstance;
}

void ResourceManager::load(const QVariantMap& data)
{
    if (data.contains("resources") && data.value("resources").type() == QVariant::Map) {
        QVariantMap resourcesMap = data.value("resources").toMap();
        QMapIterator<QString, QVariant> it(resourcesMap);
        QVariantList menusData;
        QVariantMap resData;
        QString resType;

        while(it.hasNext()) {
            it.next();
            if (it.value().type() != QVariant::Map)
                continue;

            //Temporary fix: add menus last
            resData = it.value().toMap();
            resType = resData.value("type").toString();
            if (resType == "Menu")
                menusData.append(resData);
            else
                add(resData);
        }

        foreach(const QVariant& data, menusData) {
            add(data.toMap());
        }
    }
}

QVariantMap ResourceManager::toMap()
{
    QVariantMap resourcesData;
    QList<GameObject*> resources = objects();
    for (int i=0; i < resources.size(); i++) {
        resourcesData.insert(resources.at(i)->name(), resources.at(i)->toJsonObject(false));
    }
    return resourcesData;
}

