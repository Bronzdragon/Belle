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

#include "objectgroup.h"
#include "gameobjectfactory.h"

ObjectGroup::ObjectGroup(QObject *parent,  const QString& name) :
    Object(parent, name)
{
    init();
}

ObjectGroup::ObjectGroup(const QVariantMap& data, QObject *parent) :
    Object(data, parent)
{
    init();
    loadInternal(data);
}

void ObjectGroup::loadData(const QVariantMap& data, bool internal)
{
    if (!internal)
        Object::loadData(data, internal);

    Object* obj = 0;

    if (data.value("objectsSynced").type() == QVariant::Bool) {
        setObjectsSynced(data.value("objectsSynced").toBool());
    }

    if (data.value("resizeToContentsEnabled").type() == QVariant::Bool) {
        setResizeToContentsEnabled(data.value("resizeToContentsEnabled").toBool());
    }

    if (data.contains("objects") && data.value("objects").type() == QVariant::List) {
        this->removeAllObjects(true);
        QVariantList objects = data.value("objects").toList();
        Object *obj = 0;

        for(int i=0; i < objects.size(); i++) {
            if (objects[i].type() == QVariant::Map) {
                obj = createObject(objects[i].toMap());
            }
            else if (objects[i].type() == QMetaType::QObjectStar) {
                obj = qobject_cast<Object*>(objects[i].value<QObject*>());
            }

            if(obj)
                _append(obj);
        }

        checkStickyObjects();
        updateSpacing();
    }

    //internal data passed between resource and clones
    if (data.contains("_object") && data.value("_object").type() == QVariant::Map) {
        QVariantMap object = data.value("_object").toMap();
        obj = 0;
        if (object.contains("_index") && object.value("_index").canConvert(QVariant::Int)) {
            int _index = object.value("_index").toInt();
            if (_index >= 0 && _index < mObjects.size()) {
                obj = mObjects.at(_index);
                if (obj)
                    loadObject(obj, object);
            }
        }

        if (mObjectsSynced)
            loadOtherObjects(obj, object);

        notify("_object", object);
    }

    if (data.contains("editingModeData") && data.value("editingModeData").type() == QVariant::Map) {
        QVariantMap editingModeData = data.value("editingModeData").toMap();
        QVariantList rects = editingModeData.value("rects").toList();
        QRect rect;
        int origX = x();
        int origY = y();
        Object* lastObj = 0;
        setEditingMode(true);
        for(int i=0; i < mObjects.size() && i < rects.size(); i++) {
            obj = mObjects.at(i);
            rect = rects.at(i).toRect();
            if (obj) {
                bool blocked = obj->blockSignals(true);
                obj->setX(this->x() + rect.x());
                obj->setY(this->y() + rect.y());
                obj->setWidth(rect.width());
                obj->setHeight(rect.height());
                obj->blockSignals(blocked);
                lastObj = obj;
            }
        }

        //Make sure all objects are updated.
        if (mObjectsSynced && lastObj) {
            int width = lastObj->width();
            int height = lastObj->height();

            for(int i=rects.size(); i < mObjects.size(); i++) {
                obj = mObjects.at(i);
                if (obj) {
                    bool blocked = obj->blockSignals(true);
                    obj->setWidth(width);
                    obj->setHeight(height);
                    obj->blockSignals(blocked);
                }
            }
        }

        setEditingMode(false);
        setX(origX);
        setY(origY);
    }

    if (data.contains("alignEnabled") && data.value("alignEnabled").type() == QVariant::Bool) {
        setAlignEnabled(data.value("alignEnabled").toBool());
    }

    if (data.contains("spacing") && data.value("spacing").canConvert(QVariant::Int)) {
        bool ok = false;
        int spacing = data.value("spacing").toInt(&ok);
        if (ok)
            setSpacing(spacing);
    }
}

void ObjectGroup::init()
{
    setType(GameObjectMetaType::ObjectGroup);
    mEditingMode = false;
    mSelectedObject = 0;
    mAligning = false;
    mAlignEnabled = true;
    mSpacing = 0;
    mObjectsSynced = false;
    mResizeToContentsEnabled = true;
}

void ObjectGroup::_append(Object* obj)
{
    connect(obj, SIGNAL(dataChanged(const QVariantMap&)), this, SLOT(objectChanged(const QVariantMap&)));
    connect(obj, SIGNAL(destroyed(Object*)), this, SLOT(objectDestroyed(Object*)));
    connect(obj, SIGNAL(eventActionInserted(Interaction::InputEvent,int,Action*)), this, SLOT(onObjectEventActionInserted(Interaction::InputEvent,int,Action*)));
    connect(obj, SIGNAL(eventActionRemoved(Interaction::InputEvent,Action*,bool)), this, SLOT(onObjectEventActionRemoved(Interaction::InputEvent,Action*,bool)));
    connect(obj, SIGNAL(eventActionMoved(Interaction::InputEvent,Action*,int)), this, SLOT(onObjectEventActionMoved(Interaction::InputEvent,Action*,int)));

    if (mObjects.isEmpty()) {
        initObjectEventsActions(obj);
    }
    else {
        Object* lastObj = mObjects.last();
        if (lastObj) {
            connectObjectEventsActions(lastObj, obj);
        }
    }

    ObjectGroup* resource = qobject_cast<ObjectGroup*>(this->resource());
    if (resource) {
        Object* resourceObject = resource->object(count());
        if (resourceObject) {
            connectObjectEventsActions(resourceObject, obj);
        }
    }

    mObjects.append(obj);
}

void ObjectGroup::append(Object* obj, int x)
{
    if (! obj)
        return;

    int starty =  this->height();
    if (mObjects.size())
        starty += mSpacing;
    add(obj, x, starty);
}

void ObjectGroup::add(Object* obj, int x, int y)
{
    if (! obj)
        return;

    obj->setX(this->x() + x);
    obj->setY(this->y() + y);
    _append(obj);
    adaptSize();
    this->notify("objects", variantObjects());
}

void ObjectGroup::resizeSceneRect(int x, int y)
{
    int prevX = mSceneRect.x();
    int prevY = mSceneRect.y();
    QRect rect = childrenRect();

    Object::resizeSceneRect(x, y);

    if (mAlignEnabled) {
        int minHeight = childrenMinHeight();
        int minWidth = 0;

        if (mSceneRect.width() < minWidth) {
            if (mSceneRect.x() != prevX)
                mSceneRect.setX(rect.x());
            mSceneRect.setWidth(minWidth);
        }

        if (mSceneRect.height() < minHeight) {
            if (mSceneRect.y() != prevY)
                mSceneRect.setY(rect.y());
            mSceneRect.setHeight(minHeight);
        }
    }
    else {
        if (mSceneRect.right() < rect.right()) {
            mSceneRect.setRight(rect.right());
        }

        if (mSceneRect.left() > rect.left()) {
            mSceneRect.setLeft(rect.left());
        }

        if (mSceneRect.top() > rect.top()) {
            mSceneRect.setTop(rect.top());
        }

        if (mSceneRect.bottom() < rect.bottom()) {
            mSceneRect.setBottom(rect.bottom());
        }
    }

    this->alignObjects();
}

int ObjectGroup::calcSpacing() const
{
    if (mObjects.size() < 2)
        return 0;

    int sumObjHeight = 0;
    int height = this->height();
    for(int i=0; i < mObjects.size(); i++)
        sumObjHeight += mObjects[i]->height();
    int spacing = (height - sumObjHeight) / (mObjects.size()-1);
    if (height > sumObjHeight) {
        return spacing;
    }

    return 0;
}

void ObjectGroup::setSpacing(int spacing)
{
    if (mSpacing == spacing)
        return;

    mSpacing = spacing;

    //avoid sending superfluous signals. Just send "spacing" signal at the end
    bool blocked = blockSignals(true);
    _alignObjectsVertically();
    adaptSize();
    blockSignals(blocked);
    notify("spacing", mSpacing);
}

int ObjectGroup::spacing() const
{
    return mSpacing;
}

int ObjectGroup::childrenMinHeight() const
{
    int height = 0;
    for(int i=0; i < mObjects.size(); i++) {
        height += mObjects[i]->height();
    }
    return height;
}

void ObjectGroup::alignObjects()
{
    if (!mAlignEnabled)
        return;

    mAligning = true;
    alignObjectsHorizontally();
    alignObjectsVertically();
    QVariantMap data;
    data.insert("rects", objectsRelativeRectsData());
    notify("editingModeData", data);
    mAligning = false;
}

void ObjectGroup::alignObjectsHorizontally()
{
    QRect rect = this->sceneRect(), objRect;
    int x = this->x(), objX=0, leftspace=0, objWidth=0;

    for(int i=0; i < mObjects.size(); i++) {
        objRect = mObjects[i]->sceneRect();
        objWidth = mObjects[i]->width();

        if (objRect.left() < rect.left()) {
            mObjects[i]->setX(x);
            objRect = mObjects[i]->sceneRect();
        }
        else if (rect.left() < objRect.left()) {
            if (mStickyObjects.contains(mObjects[i])) {
                mObjects[i]->setX(x);
                mObjects[i]->setWidth(this->width());
                objRect = mObjects[i]->sceneRect();
            }
        }

        if (objRect.right() > rect.right()) {
            leftspace = objRect.x() - rect.x();
            leftspace -= objRect.right() - rect.right();
            objX = x;
            if (leftspace >= 0)
                objX = x + leftspace;
            else
                objWidth -= leftspace * -1;
            mObjects[i]->setX(objX);
            mObjects[i]->setWidth(objWidth);
            checkStickyObject(mObjects[i]);
        }
        else if (objRect.right() < rect.right()) {
            if (mStickyObjects.contains(mObjects[i]))
                mObjects[i]->setWidth(this->width());
        }
    }
}

void ObjectGroup::alignObjectsVertically()
{
    updateSpacing();
    _alignObjectsVertically();
}

void ObjectGroup::_alignObjectsVertically()
{
    int starty = this->y();
    for(int i=0; i < mObjects.size(); i++) {
        mObjects[i]->setY(starty);
        starty += mObjects[i]->height() + mSpacing;
    }
}

void ObjectGroup::adaptSize()
{
    if (mAligning || mObjects.isEmpty())
        return;

    QRect rect = childrenRect();

    if (mResizeToContentsEnabled) {
        Object::setX(rect.left());
        Object::setY(rect.top());
        Object::setWidth(rect.width());
        Object::setHeight(rect.height());
    }
    else {
        if (rect.left() < mSceneRect.left())
            mSceneRect.setLeft(rect.left());

        if (rect.top() < mSceneRect.top())
            mSceneRect.setTop(rect.top());

        if (rect.right() > mSceneRect.right())
            mSceneRect.setRight(rect.right());

        if (rect.bottom() > mSceneRect.bottom())
            mSceneRect.setBottom(rect.bottom());
    }

    checkStickyObjects();
}

Object * ObjectGroup::object(int index) const
{
    if (index >= 0 && index < mObjects.size())
        return mObjects[index];

    return 0;
}

Object * ObjectGroup::object(const QString& name) const
{
    for(int i=0; i < mObjects.size(); i++)
        if (mObjects[i]->objectName() == name)
            return mObjects[i];

    return 0;
}

Object * ObjectGroup::object(const QPoint& point) const
{
    for(int i=mObjects.size()-1; i >=0; --i)
        if (mObjects[i]->contains(point.x(), point.y()))
            return mObjects[i];

    return 0;
}

void ObjectGroup::move(int x, int y)
{
    if (mEditingMode)
        return;

    QRect prevSceneRect = sceneRect();
    int difX, difY;
    Object::move(x, y);

    difX = sceneRect().topLeft().x() - prevSceneRect.topLeft().x();
    difY = sceneRect().topLeft().y() - prevSceneRect.topLeft().y();

    for(int i=mObjects.size()-1; i >=0; --i) {
        mObjects[i]->move( mObjects[i]->x()+difX,  mObjects[i]->y()+difY);
    }
}

void ObjectGroup::paint(QPainter & painter)
{
    Object::paint(painter);
    if (! painter.opacity())
        return;

    for(int i=0; i < mObjects.size(); i++) {
        painter.save();
        mObjects[i]->paint(painter);
        painter.restore();
    }
}

QList<Object*> ObjectGroup::objects() const
{
    return mObjects;
}

QVariantList ObjectGroup::variantObjects() const
{
    QVariantList objects;
    for(int i=0; i < mObjects.size(); i++) {
        objects.append(QVariant::fromValue(qobject_cast<QObject*>(mObjects[i])));
    }
    return objects;
}

Object* ObjectGroup::objectAt(qreal x, qreal y)
{
    if (mEditingMode) {
        for(int i=mObjects.size()-1; i >= 0; i--) {
            if (mObjects[i]->contains(x, y)) {
                return mObjects[i];
            }
        }
    }
    if (this->contains(x, y))
        return this;
    return 0;
}

void ObjectGroup::setX(int x)
{
    int move = x - this->x();
    //FIXME: Only works for vertical layouts.
    for(int i=mObjects.size()-1; i >=0; --i) {
        mObjects[i]->setX(mObjects[i]->x()+move);
    }

    Object::setX(x);
}

void ObjectGroup::setY(int y)
{
    int move = y - this->y();
    for(int i=0; i < mObjects.size(); i++) {
        mObjects[i]->setY(mObjects[i]->y()+move);
    }

    Object::setY(y);
}

void ObjectGroup::setWidth(int width, bool percent)
{
    if (!percent && !mAlignEnabled) {
        QRect rect = childrenRect();
        if (width < rect.width())
            width = rect.width();
    }

    Object::setWidth(width, percent);

    this->alignObjects();
}

void ObjectGroup::setHeight(int height, bool percent)
{
    if (!percent) {
        int minHeight = 0;
        if (mAlignEnabled)
            minHeight = childrenMinHeight();
        else
            minHeight = childrenRect().height();

        if (height < minHeight)
            height = minHeight;
    }

    Object::setHeight(height, percent);

    this->alignObjects();
}

bool ObjectGroup::editingMode() const
{
    return mEditingMode;
}

void ObjectGroup::setEditingMode(bool mode)
{
    if (mEditingMode == mode)
        return;

    mEditingMode = mode;
    if (!mEditingMode) {
        bool blocked = blockSignals(true);
        adaptSize();
        blockSignals(blocked);
        QVariantMap data;
        data.insert("rects", objectsRelativeRectsData());
        notify("editingModeData", data);
    }
    notify("editingMode", mEditingMode);
}

QVariantList ObjectGroup::objectsRelativeRectsData()
{
    QVariantList data;
    QRect rect;

    foreach(Object* obj, mObjects) {
        rect = obj->sceneRect();
        rect.moveLeft(rect.x() - this->x());
        rect.moveTop(rect.y() - this->y());
        data.append(rect);
    }

    return data;
}

QList<QRect> ObjectGroup::objectsRelativeRects() const
{
    QList<QRect> rects;
    QRect rect;

    foreach(Object* obj, mObjects) {
        rect = obj->sceneRect();
        rect.moveLeft(rect.x() - this->x());
        rect.moveTop(rect.y() - this->y());
        rects.append(rect);
    }

    return rects;
}

void ObjectGroup::checkStickyObjects()
{
    mStickyObjects.clear();
    for(int i=0; i < mObjects.size(); i++) {
        if (mObjects[i]->width() == this->width())
            mStickyObjects.append(mObjects[i]);
    }
}

void ObjectGroup::checkStickyObject(Object * obj)
{
    if (obj->width() == this->width() && ! mStickyObjects.contains(obj)) {
        mStickyObjects.append(obj);
    }
}

void ObjectGroup::removeObjectAt(int index, bool del)
{
    if (index >= 0 && index < mObjects.size()) {
        Object* obj = mObjects.takeAt(index);
        this->notify("objects", variantObjects());
        obj->disconnect(this);
        if (del)
            obj->deleteLater();
        adaptSize();
    }
}

void ObjectGroup::removeAllObjects(bool del)
{
    bool signalsAlreadyBlocked = this->signalsBlocked();
    //don't emit a signal for each removed object
    //instead emit a signal with empty list
    if (! signalsAlreadyBlocked)
        this->blockSignals(true);
    for(int i=mObjects.size()-1; i >= 0; i--) {
        this->removeObjectAt(i, del);
    }
    this->blockSignals(signalsAlreadyBlocked);
    this->notify("objects", QVariantList());
}


int ObjectGroup::indexOf(Object * object)
{
    if (object) {
        return mObjects.indexOf(object);
    }

    return -1;
}

void ObjectGroup::objectDestroyed(Object * object)
{
    int index = mObjects.indexOf(object);
    if (index != -1)
        this->removeObjectAt(index);
}

void ObjectGroup::prepareObjectData(QVariantMap & data)
{
    //size and position properties will be set later using editingModeData signal
    if (data.contains("width"))
        data.remove("width");

    if (data.contains("height"))
        data.remove("height");

    if (data.contains("x"))
        data.remove("x");

    if (data.contains("y"))
        data.remove("y");
}

void ObjectGroup::objectChanged(const QVariantMap& data)
{
    Object * sender = qobject_cast<Object*>(this->sender());
    int index = this->indexOf(sender);

    if (index != -1) {
        QVariantMap object = data;
        prepareObjectData(object);
        if (!object.isEmpty()) {
            object.insert("_index", index);
            this->notify("_object", object);
        }

        if (mObjectsSynced)
            loadOtherObjects(sender, data);

        if (mEditingMode && (data.contains("x") || data.contains("y") || data.contains("width") || data.contains("height"))) {
            bool blocked = blockSignals(true);
            adaptSize();
            blockSignals(blocked);
        }
    }
}

QVariantMap ObjectGroup::toJsonObject(bool internal) const
{
    QVariantMap object = Object::toJsonObject(internal);
    QVariantList objects;
    QVariantMap objData;
    int x = this->x();
    int y = this->y();

    for(int i=0; i < mObjects.size(); i++) {
        objData = mObjects[i]->toJsonObject(internal);
        if (internal) {
            objData.insert("relativeX", mObjects[i]->x() - x);
            objData.insert("relativeY", mObjects[i]->y() - y);
        }
        objects.append(objData);
    }

    if (! objects.isEmpty())
        object.insert("objects", objects);

    if (internal)
        object.insert("spacing", mSpacing);

    object.insert("alignEnabled", mAlignEnabled);
    object.insert("resizeToContentsEnabled", mResizeToContentsEnabled);
    object.insert("objectsSynced", mObjectsSynced);

    return object;
}

Object* ObjectGroup::createObject(const QVariantMap & data)
{
    Object* obj = ResourceManager::instance()->createObject(data, this);
    int x = this->x();
    int y = this->y();

    if (obj) {
        if (data.contains("relativeX") && data.value("relativeX").canConvert(QVariant::Int))
            obj->setX(x + data.value("relativeX").toInt());

        if (data.contains("relativeY") && data.value("relativeY").canConvert(QVariant::Int))
            obj->setY(y + data.value("relativeY").toInt());
    }

    return obj;
}

void ObjectGroup::updateSpacing()
{
    mSpacing = calcSpacing();
    notify("spacing", mSpacing);
}

void ObjectGroup::adaptLayout()
{
    if (mAligning)
        return;

    adaptSize();
    updateSpacing();
}

void ObjectGroup::loadObject(Object * obj, const QVariantMap & data)
{
    if (!obj)
        return;

    obj->load(data);
    bool blocked = obj->blockNotifications(true);
    //some properties are ignored by default so we need to apply them here for the child objects
    if (data.contains("visible") && data.value("visible").type() == QVariant::Bool)
        obj->setVisible(data.value("visible").toBool());
    obj->blockNotifications(blocked);
}

void ObjectGroup::loadOtherObjects(Object * object, const QVariantMap & data)
{
    if (data.isEmpty())
        return;

    QVariantMap _data = data;
    _data.remove("relativeX");
    _data.remove("relativeY");

    foreach(Object* obj, mObjects) {
        if (obj != object)
            loadObject(obj, _data);
    }
}

bool ObjectGroup::isAlignEnabled() const
{
    return mAlignEnabled;
}

void ObjectGroup::setAlignEnabled(bool enabled)
{
    if (mAlignEnabled == enabled)
        return;

    mAlignEnabled = enabled;
    notify("alignEnabled", mAlignEnabled);
}

QRect ObjectGroup::childrenRect() const
{
    if (mObjects.isEmpty())
        return QRect();

    int h = 0;
    int w = width();

    QRect rect = mObjects.first()->sceneRect();
    int top = rect.top();
    int left = rect.left();
    int right = rect.right();
    int bottom = rect.bottom();

    for(int i=1; i < mObjects.size(); i++) {
        rect = mObjects[i]->sceneRect();
        if (rect.top() < top)
            top = rect.top();
        if (rect.left() < left)
            left = rect.left();
        if (rect.right() > right)
            right = rect.right();
        if (rect.bottom() > bottom)
            bottom = rect.bottom();
    }

    //Due to Qt's peculiar behaviour we need to add 1 to right/bottom
    w = (right+1) - left;
    h = (bottom+1) - top;

    return QRect(left, top, w, h);
}

int ObjectGroup::count() const
{
    return mObjects.size();
}

bool ObjectGroup::objectsSynced() const
{
    return mObjectsSynced;
}

void ObjectGroup::setObjectsSynced(bool sync)
{
    if (mObjectsSynced == sync)
        return;

    mObjectsSynced = sync;
    notify("objectsSynced", mObjectsSynced);
    emit objectsSyncChanged(mObjectsSynced);
}

void ObjectGroup::connectToResource()
{
    ObjectGroup * resource = qobject_cast<ObjectGroup*>(this->resource());
    if (resource) {
        setObjectsSynced(resource->objectsSynced());
        connect(resource, SIGNAL(objectEventActionInserted(int,Interaction::InputEvent,int,Action*, ActionPool*)),
                this, SLOT(onObjectEventActionInsertedSync(int,Interaction::InputEvent,int,Action*, ActionPool*)));
        connect(this, SIGNAL(objectEventActionRemoved(int,Interaction::InputEvent,Action*,bool)),
                resource, SLOT(onObjectEventActionRemovedSync(int,Interaction::InputEvent,Action*,bool)), Qt::UniqueConnection);
        connect(resource, SIGNAL(objectEventActionRemoved(int,Interaction::InputEvent,Action*,bool)),
                this, SLOT(onObjectEventActionRemovedSync(int,Interaction::InputEvent,Action*,bool)), Qt::UniqueConnection);
        connect(this, SIGNAL(objectEventActionMoved(int,Interaction::InputEvent,Action*,int)),
                resource, SLOT(onObjectEventActionMovedSync(int,Interaction::InputEvent,Action*,int)), Qt::UniqueConnection);
        connect(resource, SIGNAL(objectEventActionMoved(int,Interaction::InputEvent,Action*,int)),
                this, SLOT(onObjectEventActionMovedSync(int,Interaction::InputEvent,Action*,int)), Qt::UniqueConnection);

        addActionPools(resource->mObjectsActionPools);

        Action* centralAction;
        foreach(Action* action, resource->mObjectsEventActions) {
            centralAction = createObjectsEventAction(action, actionPoolFromAction(action));
            if (centralAction)
                centralAction->setResource(action);
        }

        int count = this->count();
        int resourceCount = resource->count();

        for (int i=0; i < count && i < resourceCount; i++) {
            connectObjectEventsActions(resource->object(i), mObjects.at(i));
        }

        if (count > resourceCount) {
            for (int i=resourceCount; i < count; i++) {
                if (i > 0) {
                    connectObjectEventsActions(mObjects.at(i-1), mObjects.at(i));
                }
            }
        }

        cleanupObjectsEventActions();
    }

    Object::connectToResource();
}

void ObjectGroup::onObjectEventActionInserted(Interaction::InputEvent event, int index, Action * action)
{
    if (loadBlocked() || !action)
        return;

    Object* sender = qobject_cast<Object*>(this->sender());
    if (!sender)
        return;

    int objectIndex = mObjects.indexOf(sender);
    if (objectIndex == -1)
        return;

    ActionPool* actionPool = 0;

    if (mObjectsSynced) {
        actionPool = createActionPool();
        actionPool->addAction(action);

        Action* centralAction = createObjectsEventAction(action, actionPool);
        action->setResource(centralAction);
        connect(this, SIGNAL(objectsSyncChanged(bool)), action, SLOT(setSync(bool)), Qt::UniqueConnection);

        QList<Object*> objects = mObjects;
        objects.removeAll(sender);
        insertEventActionInObjects(objects, event, index, centralAction, actionPool);
    }

    bool blocked = blockLoad(true);
    emit objectEventActionInserted(objectIndex, event, index, action, actionPool);
    blockLoad(blocked);
}

//This method will be called only in clones
void ObjectGroup::onObjectEventActionInsertedSync(int objectIndex, Interaction::InputEvent event, int index, Action * action, ActionPool* actionPool)
{
    if (!action)
        return;

    if (mObjectsSynced) {
        Action* centralAction = createObjectsEventAction(action, actionPool);
        GameObject* resource = action->resource();
        if (!resource)
            resource = action;
        centralAction->setResource(resource);
        connect(this, SIGNAL(syncChanged(bool)), centralAction, SLOT(setSync(bool)), Qt::UniqueConnection);
        insertEventActionInObjects(mObjects, event, index, centralAction, actionPool);
    }
    else {
        Object* obj = this->object(objectIndex);
        if (obj) {
            Action* newAction = GameObjectFactory::createAction(action->toJsonObject(), obj);
            newAction->setResource(action);
            connect(this, SIGNAL(syncChanged(bool)), newAction, SLOT(setSync(bool)), Qt::UniqueConnection);
            if (actionPool)
                actionPool->addAction(newAction);
            obj->insertEventAction(event, index, newAction);
        }
    }

    addActionPool(actionPool);
}

void ObjectGroup::insertEventActionInObjects(const QList<Object *> & objects, Interaction::InputEvent event, int index, Action* action, ActionPool* actionPool)
{
    bool blocked;
    Action* newAction = 0;

    if (!action || objects.isEmpty())
        return;

    foreach(Object* obj, objects) {
        blocked = obj->blockSignals(true);
        newAction = GameObjectFactory::createAction(action->toJsonObject(), obj);
        newAction->setResource(action);
        connect(this, SIGNAL(objectsSyncChanged(bool)), newAction, SLOT(setSync(bool)), Qt::UniqueConnection);
        if (actionPool)
            actionPool->addAction(newAction);
        obj->insertEventAction(event, index, newAction);
        obj->blockSignals(blocked);
    }
}

Action* ObjectGroup::createObjectsEventAction(Action * action, ActionPool* pool)
{
    if (!action)
        return 0;

    Action* centralAction = GameObjectFactory::createAction(action->toJsonObject(), this);
    mObjectsEventActions.append(centralAction);
    if (pool)
        pool->addAction(centralAction);
    return centralAction;
}

ActionPool* ObjectGroup::createActionPool()
{
    ActionPool* pool = new ActionPool(this);
    addActionPool(pool);
    return pool;
}

void ObjectGroup::addActionPool(ActionPool* pool)
{
    if (!pool)
        return;

    mObjectsActionPools.append(pool);
    connect(pool, SIGNAL(destroyed(ActionPool*)), this, SLOT(onActionPoolDestroyed(ActionPool*)));
}

void ObjectGroup::addActionPools(const QList<ActionPool *> & pools)
{
    foreach(ActionPool* pool, pools) {
        addActionPool(pool);
    }
}

ActionPool* ObjectGroup::actionPoolFromAction(Action * action)
{
    if (!action)
        return 0;

    foreach(ActionPool* pool, mObjectsActionPools) {
        if (pool->contains(action))
            return pool;
    }

    return 0;
}

void ObjectGroup::onObjectEventActionRemoved(Interaction::InputEvent event, Action * action, bool del)
{
    if (loadBlocked() || !action)
        return;

    Object* sender = qobject_cast<Object*>(this->sender());
    if (!sender)
        return;

    int objectIndex = mObjects.indexOf(sender);
    if (objectIndex == -1)
        return;

    if (mObjectsSynced) {
        ActionPool* pool = actionPoolFromAction(action);
        if (pool) {
            removeObjectsEventActionsFromPool(event, pool, del);

            Action* resourceAction = qobject_cast<Action*>(action->resource());
            if (resourceAction && mObjectsEventActions.contains(resourceAction)) {
                mObjectsEventActions.removeAll(resourceAction);
                delete resourceAction;
            }
        }
    }

    bool loadBlocked = blockLoad(true);
    emit objectEventActionRemoved(objectIndex, event, action, del);
    blockLoad(loadBlocked);
}

void ObjectGroup::onObjectEventActionRemovedSync(int objectIndex, Interaction::InputEvent event, Action * action, bool del)
{
    if (loadBlocked() || !action)
        return;

    Object* object = this->object(objectIndex);
    ActionPool* pool = actionPoolFromAction(action);
    Action* objectAction = 0;

    if (pool) {
        if (object) {
            objectAction = pool->actionFromParent(object);
            object->removeEventAction(event, objectAction, del);
        }
        else if (mObjectsSynced) {
            removeObjectsEventActionsFromPool(event, pool, del);
        }
    }
    else if (object) {
        object->removeEventActionSync(event, action, del);
    }
}

void ObjectGroup::onActionPoolDestroyed(ActionPool * pool)
{
    if (mObjectsActionPools.contains(pool)) {
        mObjectsActionPools.removeAll(pool);
    }
}

void ObjectGroup::removeObjectsEventActionsFromPool(Interaction::InputEvent event, ActionPool* pool, bool del)
{
    if (!pool)
        return;

    bool blocked;
    Action* objectAction;

    foreach(Object* obj, mObjects) {
        objectAction = pool->actionFromParent(obj);
        if (objectAction) {
            blocked = obj->blockSignals(true);
            obj->removeEventAction(event, objectAction, del);
            obj->blockSignals(blocked);
        }
    }
}

void ObjectGroup::connectObjectEventActions(Interaction::InputEvent event, Object * source, Object * target)
{
    if (!target || !source)
        return;

    QList<Action*> sourceActions, targetActions;
    sourceActions = source->actionsForEvent(event);
    targetActions = target->actionsForEvent(event);
    Action* sourceAction;
    Action* targetAction;
    GameObject* resourceAction = 0;
    ActionPool* pool = 0;
    ActionPool* oldPool = 0;
    bool resourceConnect = false;
    bool directResourceConnect = false;

    if (source->isResource() && !target->isResource())
        resourceConnect = true;

    for(int i=0; i < sourceActions.size() && i < targetActions.size(); i++) {
        sourceAction = sourceActions.at(i);
        targetAction = targetActions.at(i);
        if (!sourceAction || !targetAction)
            break;

        if (sourceAction->type() != targetAction->type())
            break;

        directResourceConnect = false;
        resourceAction = 0;
        pool = actionPoolFromAction(sourceAction);
        oldPool = actionPoolFromAction(targetAction);
        if (pool) {
            resourceAction = pool->actionFromParent(this);
            if (oldPool != pool) {
                if (oldPool)
                    oldPool->removeAction(targetAction);
                pool->addAction(targetAction);
            }
        }

        if (resourceConnect && !resourceAction) {
            directResourceConnect = true;
            if (sourceAction->resource()) {
                resourceAction = sourceAction->resource();
            }
            else {
                resourceAction = sourceAction;
            }
        }

        if (resourceAction) {
            targetAction->setResource(resourceAction);
            targetAction->setSync(sourceAction->isSynced());

            if (directResourceConnect)
                connect(this, SIGNAL(syncChanged(bool)), targetAction, SLOT(setSync(bool)), Qt::UniqueConnection);
            else
                connect(this, SIGNAL(objectsSyncChanged(bool)), targetAction, SLOT(setSync(bool)), Qt::UniqueConnection);
        }
    }
}

void ObjectGroup::connectObjectEventsActions(Object * source, Object * target)
{
    connectObjectEventActions(Interaction::MouseMove, source, target);
    connectObjectEventActions(Interaction::MousePress, source, target);
    connectObjectEventActions(Interaction::MouseRelease, source, target);
}

void ObjectGroup::initObjectEventActions(Interaction::InputEvent event, Object * object)
{
    if (!object || !mObjectsSynced)
        return;

    QList<Action*> actions = object->actionsForEvent(event);
    if (actions.isEmpty())
        return;

    ActionPool* pool;
    Action* resourceAction;

    foreach(Action* action, actions) {
        pool = createActionPool();
        resourceAction = createObjectsEventAction(action, pool);
        pool->addAction(action);

        if (resourceAction) {
            action->setResource(resourceAction);
            connect(this, SIGNAL(objectsSyncChanged(bool)), action, SLOT(setSync(bool)), Qt::UniqueConnection);
        }
    }
}

void ObjectGroup::initObjectEventsActions(Object * object)
{
    initObjectEventActions(Interaction::MouseMove, object);
    initObjectEventActions(Interaction::MousePress, object);
    initObjectEventActions(Interaction::MouseRelease, object);
}

void ObjectGroup::cleanupObjectsEventActions()
{
    Action* action;

    for(int i=mObjectsEventActions.size()-1; i >= 0; i--) {
        action = mObjectsEventActions.at(i);
        if (action && action->parent() == this && action->clones().isEmpty()) {
            mObjectsEventActions.removeAt(i);
            delete action;
        }
    }
}

void ObjectGroup::onObjectEventActionMoved(Interaction::InputEvent event, Action * action, int index)
{
    if (loadBlocked() || !action)
        return;

    Object* sender = qobject_cast<Object*>(this->sender());
    if (!sender)
        return;

    int objectIndex = mObjects.indexOf(sender);
    if (objectIndex == -1)
        return;

    if (mObjectsSynced) {
        ActionPool* pool = actionPoolFromAction(action);
        if (pool) {
            Action* objectAction;
            bool blocked;
            foreach(Object* obj, mObjects) {
                objectAction = pool->actionFromParent(obj);
                if (objectAction) {
                    blocked = obj->blockSignals(true);
                    obj->moveEventAction(event, objectAction, index);
                    obj->blockSignals(blocked);
                }
            }
        }
    }

    bool loadBlocked = blockLoad(true);
    emit objectEventActionMoved(objectIndex, event, action, index);
    blockLoad(loadBlocked);
}

void ObjectGroup::onObjectEventActionMovedSync(int objectIndex, Interaction::InputEvent event, Action *action, int index)
{
    if (loadBlocked() || !action)
        return;

    Object* object = this->object(objectIndex);
    ActionPool* pool = actionPoolFromAction(action);
    Action* objectAction = 0;

    if (pool) {
        if (object) {
            objectAction = pool->actionFromParent(object);
            object->moveEventAction(event, objectAction, index);
        }
        else if (mObjectsSynced) {
            moveObjectsEventActionsFromPool(event, pool, index);
        }
    }
    else if (object) {
        object->moveEventActionSync(event, action, index);
    }
}

void ObjectGroup::moveObjectsEventActionsFromPool(Interaction::InputEvent event, ActionPool * pool, int index)
{
    if (!pool)
        return;

    bool blocked;
    Action* objectAction;

    foreach(Object* obj, mObjects) {
        objectAction = pool->actionFromParent(obj);
        if (objectAction) {
            blocked = obj->blockSignals(true);
            obj->moveEventAction(event, objectAction, index);
            obj->blockSignals(blocked);
        }
    }
}

bool ObjectGroup::resizeToContentsEnabled() const
{
    return mResizeToContentsEnabled;
}

void ObjectGroup::setResizeToContentsEnabled(bool enabled)
{
    mResizeToContentsEnabled = enabled;
}
