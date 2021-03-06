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

#ifndef CHANGE_VISIBILITY_H
#define CHANGE_VISIBILITY_H

#include "action.h"
#include "character.h"
#include "fade.h"
#include "slide.h"

class ChangeVisibilityEditorWidget;
class HideEditorWidget;
class ShowEditorWidget;
class Fade;
class Slide;

class ChangeVisibility : public Action
{
    Q_OBJECT

    bool mToShow;
    Fade* mFadeAction;
    Slide* mSlideAction;

public:
    ChangeVisibility(bool, QObject *parent = 0);
    ChangeVisibility(const QVariantMap& data, QObject *parent);
    virtual QString displayText() const;
    void setCharacter(Character*);
    Character* character() const;
    virtual QVariantMap toJsonObject(bool internal=true) const;
    bool toShow() const;
    bool toHide() const;
    virtual void setSceneObject(Object *);

    void setFadeActionEnabled(bool);
    bool isFadeActionEnabled() const;
    void setFadeAction(Fade*);
    Fade* fadeAction() const;

    void setSlideActionEnabled(bool);
    bool isSlideActionEnabled() const;
    void setSlideAction(Slide*);
    Slide* slideAction() const;

protected:
    void setToShow(bool);
    virtual void loadData(const QVariantMap&, bool internal=false);
    virtual void connectToResource();

private:
    void init(bool);

signals:
    
public slots:
    
};


#endif // ChangeVisibility_H
