#ifndef GAMEOBJECTCOMBOBOX_H
#define GAMEOBJECTCOMBOBOX_H

#include <QComboBox>

#include "gameobject.h"

class GameObject;

class GameObjectComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit GameObjectComboBox(QWidget *parent = 0);
    void addObjects(const QList<GameObject*>&);
    void setObjects(const QList<GameObject*>&, GameObject* selectedObject=0);
    void insertObject(int, GameObject*);
    void addObject(GameObject*);
    void clear();
    void setCurrentObject(GameObject*);
    GameObject* currentObject() const;
    void removeObject(GameObject*);
    void removeItem(int);

    void setIconsEnabled(bool);
    bool hasIconsEnabled() const;

    void setTypeFilter(GameObjectMetaType::Type);
    GameObjectMetaType::Type typeFilter() const;

    void setTypeFilterActive(bool);
    bool isTypeFilterActive() const;

protected:

signals:
    void objectChanged(GameObject*);

private slots:
    void indexChanged(int);
    void objectDestroyed(GameObject*);

private:
    bool mIconsEnabled;
    bool mTypeFilterActive;
    GameObjectMetaType::Type mTypeFilter;

};

#endif // GAMEOBJECTCOMBOBOX_H
