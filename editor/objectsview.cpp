#include "objectsview.h"

#include <QDebug>
#include <QHeaderView>

ObjectsView::ObjectsView(QWidget *parent) :
    PropertiesWidget(parent, 1)
{
    this->setHeaderHidden(true);
    this->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(itemClicked(const QModelIndex&)));
}

QStandardItem* ObjectsView::itemFromObject(Object* object)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
    int rows = this->model()->rowCount();

    for(int i=0; i < rows; i++) {
        QStandardItem* item = model->item(i);
        if (item->hasChildren()) {
            for(int j=0; j < item->rowCount(); j++) {
                QStandardItem* child = item->child(j);
                if (child->data().isValid()) {
                    Object* obj = (Object*) child->data().value<void*>();
                    if (obj == object)
                        return child;
                }
            }
        }
    }

    return 0;
}

void ObjectsView::addObject(Object * object)
{
    if (! object)
        return;

    const GameObjectMetaType* metatype = GameObjectMetaType::metaType(object->type());
    QString typeName = metatype ? metatype->name() : "";

    if (containsGroup(typeName)) {
        setLastItem(typeName);
        appendRow(QIcon(), object->objectName());
    }
    else {
        beginGroup(typeName);
        appendRow(QIcon(), object->objectName());
        endGroup();
    }

    QStandardItem* item = lastItem();
    int rowCount = item->rowCount() ? item->rowCount()-1 : 0;
    item = item->child(rowCount);
    item->setEditable(false);
    item->setData(QVariant::fromValue((void*) object));
    connect(object, SIGNAL(nameChanged(const QString&)), this, SLOT(objectNameChanged(const QString&)));
    connect(object, SIGNAL(destroyed(QObject*)), this, SLOT(removeObject(QObject*)));
}

void ObjectsView::addObjects(const QList<Object*>& objects)
{
    for(int i=0; i < objects.size(); i++)
        addObject(objects.at(i));
}

void ObjectsView::removeObject(Object * object)
{
    if (! object)
        return;

    QStandardItem* item = itemFromObject(object);
    this->removeItem(item);
}

void ObjectsView::removeObject(QObject * qobject)
{
    removeObject(qobject_cast<Object*>(qobject));
}

void ObjectsView::itemClicked(const QModelIndex & index)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
    QStandardItem* item = model->itemFromIndex(index);
    Object* obj = (Object*) item->data().value<void*>();
    if (obj)
        emit objectSelected(obj);
}

void ObjectsView::objectNameChanged(const QString & name)
{
    Object* object = qobject_cast<Object*>(sender());
    if (object) {
        QStandardItem* item = itemFromObject(object);
        if (item) {
            item->setData(name, Qt::DisplayRole);
            update(item->index());
        }
    }
}
