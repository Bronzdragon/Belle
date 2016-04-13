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

#include "change_visibility.h"
#include "show.h"
#include "gameobjectfactory.h"

ChangeVisibility::ChangeVisibility(bool show, QObject *parent) :
    Action(parent)
{
    init(show);
}

ChangeVisibility::ChangeVisibility(const QVariantMap& data, QObject *parent) :
    Action(data, parent)
{
    QString type("Show");
    if (data.contains("type") && data.value("type").type() == QVariant::String)
        type = data.value("type").toString();

    if (type == "Hide")
        init(false);
    else
        init(true);

    if (data.contains("transitions") && data.value("transitions").type() == QVariant::List ) {
        QVariantList transitions = data.value("transitions").toList();
        for(int i=0; i <transitions.size(); i++ ) {
            if (transitions[i].type() != QVariant::Map)
                continue;
            QVariantMap data = transitions[i].toMap();
            if (data.contains("type") && data.value("type") == "Fade")
                mFadeAction = qobject_cast<Fade*>(GameObjectFactory::createAction(data, this));
            if (data.contains("type") && data.value("type") == "Slide")
                mSlideAction = qobject_cast<Slide*>(GameObjectFactory::createAction(data, this));
        }
    }
}

void ChangeVisibility::init(bool show)
{
    mToShow = show;
    mFadeAction = 0;
    mSlideAction = 0;
    setDescription(name() + "...");
    setSupportedEvents(Interaction::MousePress | Interaction::MouseRelease |
    Interaction::MouseMove);
}

QString ChangeVisibility::displayText() const
{
    if (! sceneObject())
        return "";

    QString text("");
    if (sceneObject())
        text += sceneObject()->objectName() + " ";

    Character* character = qobject_cast<Character*>(sceneObject());
    if (character && toShow()) {
        const Show* _this = qobject_cast<const Show*>(this);
        if (_this) {
            text += QString("(%1) ").arg(_this->characterState());
        }
    }

    const Fade* fade = fadeAction();
    const Slide* slide = slideAction();

    double duration = 0.0;

    if (fade && fade->duration() || slide && slide->duration()) {
        text += tr("with") + " ";

        if (fade && fade->duration()) {
            text += QString("%1%2").arg("fade").arg(fade->fadeTypeString());
            if (fade->duration() > duration)
                duration = fade->duration();
        }

        if (fade && fade->duration() && slide && slide->duration())
            text += " " + tr("and") + " ";

        if (slide && slide->duration()) {
            text += tr("slide");
            if (slide->duration() > duration)
                duration = slide->duration();
        }
        text += QString(" %1 %2s").arg(tr("in")).arg(QString::number(duration,'f', 2));
    }

    return text;
}

void ChangeVisibility::setSceneObject(Object* obj)
{
    if (! obj)
        return;

    Action::setSceneObject(obj);
    if (qobject_cast<Character*>(obj))
        setDisplayText(obj->objectName() + " (" + qobject_cast<Character*>(obj)->currentState() + ")");
    else
        setDisplayText(obj->objectName());
    if (mFadeAction)
        mFadeAction->setSceneObject(obj);
    if (mSlideAction)
        mSlideAction->setSceneObject(obj);
    emit dataChanged();
}

bool ChangeVisibility::toShow() const
{
    return mToShow;
}

bool ChangeVisibility::toHide() const
{
    return ! mToShow;
}

void ChangeVisibility::setFadeActionEnabled(bool enable)
{
    if (enable) {
        if (!mFadeAction) {
            Action* fadeAction = GameObjectFactory::createAction(GameObjectMetaType::Fade, this);
            mFadeAction = static_cast<Fade*>(fadeAction);
            mFadeAction->setDuration(0);
            connect(mFadeAction, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()), Qt::UniqueConnection);
            if (mToShow) {
                setType(GameObjectMetaType::Show);
                mFadeAction->setFadeType(Fade::In);
            }
            else {
                setType(GameObjectMetaType::Hide);
                mFadeAction->setFadeType(Fade::Out);
            }
        }
    }
    else if (mFadeAction) {
        mFadeAction->deleteLater();
        mFadeAction = 0;
    }
}

bool ChangeVisibility::isFadeActionEnabled() const
{
    return mFadeAction ? true: false;
}

void ChangeVisibility::setFadeAction(Fade* action)
{
    mFadeAction = action;
}

Fade* ChangeVisibility::fadeAction() const
{
    return mFadeAction;
}

void ChangeVisibility::setSlideActionEnabled(bool enable)
{
    if (enable) {
        if (! mSlideAction) {
            Action* slideAction = GameObjectFactory::createAction(GameObjectMetaType::Slide, this);
            mSlideAction = static_cast<Slide*>(slideAction);
            mSlideAction->setDuration(0);
            connect(mSlideAction, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()), Qt::UniqueConnection);
        }
    }
    else if (mSlideAction) {
        mSlideAction->deleteLater();
        mSlideAction = 0;
    }
}

bool ChangeVisibility::isSlideActionEnabled() const
{
    return mSlideAction ? true : false;
}

void ChangeVisibility::setSlideAction(Slide* action)
{
    mSlideAction = action;
}

Slide* ChangeVisibility::slideAction() const
{
    return mSlideAction;
}

QVariantMap ChangeVisibility::toJsonObject(bool internal) const
{
    QVariantMap object = Action::toJsonObject(internal);
    QVariantList transitions;
    if (mSlideAction)
        transitions.append(mSlideAction->toJsonObject(internal));
    if (mFadeAction)
        transitions.append(mFadeAction->toJsonObject(internal));
    object.insert("transitions", transitions);

    return object;
}
