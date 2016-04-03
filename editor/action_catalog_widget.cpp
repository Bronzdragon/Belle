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

#include "action_catalog_widget.h"

#include <QDebug>

#include "wait.h"
#include "dialogue.h"
#include "label.h"
#include "gotolabel.h"
#include "gotoscene.h"
#include "slide.h"
#include "fade.h"
#include "show.h"
#include "hide.h"
#include "branch.h"
#include "get_user_input.h"
#include "changecolor.h"
#include "playsound.h"
#include "stopsound.h"
#include "show_menu.h"
#include "end.h"
#include "change_game_variable.h"
#include "change_background.h"
#include "runscript.h"
#include "changestate.h"
#include "gameobjectfactory.h"

ActionCatalogWidget::ActionCatalogWidget(QWidget *parent) :
    PropertiesWidget(parent, 1)
{
    this->setHeaderHidden(true);

    beginGroup(tr("Story"));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::Dialogue));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::Wait));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::Show));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::Hide));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::ChangeBackground));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::ChangeState));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::Label));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::GoToLabel));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::GoToScene));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::Branch));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::ShowMenu));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::End));
    endGroup();

    beginGroup(tr("Transform"));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::Slide));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::Fade));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::ChangeColor));
    endGroup();

    beginGroup(tr("Audio"));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::PlaySound));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::StopSound));
    endGroup();

    beginGroup(tr("Scripting"));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::GetUserInput));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::ChangeGameVariable));
    _appendRow(GameObjectMetaType::metaType(GameObjectMetaType::RunScript));
    endGroup();

    setIconSize(QSize(22, 22));

    connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onDoubleClick(const QModelIndex&)));
}


void ActionCatalogWidget::onDoubleClick(const QModelIndex & index)
{
    if (! index.parent().isValid())
        return;

    const QStandardItemModel* model = qobject_cast<const QStandardItemModel*> (index.model());
    if (! model)
        return;

    QVariant data = index.data(Qt::UserRole+1);
    bool ok;
    int type = data.toInt(&ok);
    if (! ok)
        return;

    Action* action = GameObjectFactory::createAction((GameObjectMetaType::Type) type);

    if (action)
        emit newAction(action);
}

void ActionCatalogWidget::_appendRow(const GameObjectMetaType * metaType)
{
    if (metaType) {
        appendRow(metaType->icon(), metaType->name(), metaType->type());
    }
}
