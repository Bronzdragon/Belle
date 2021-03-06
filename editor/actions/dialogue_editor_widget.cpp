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

#include "dialogue_editor_widget.h"

#include "character.h"
#include "dialoguebox.h"

#include <QCompleter>

DialogueEditorWidget::DialogueEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mChooseCharacterWidget = new ObjectComboBox(this);
    mChooseCharacterWidget->addTypeFilter(GameObjectMetaType::Character);
    mChooseTextBoxWidget = new ObjectComboBox(this);
    mChooseTextBoxWidget->addTypeFilter(GameObjectMetaType::TextBox);
    mChooseTextBoxWidget->addTypeFilter(GameObjectMetaType::DialogueBox);
    mChooseCharacterWidget->setEditable(true);
    if (mChooseCharacterWidget->completer())
        mChooseCharacterWidget->completer()->setCompletionMode(QCompleter::PopupCompletion);
    mTextEdit = new QTextEdit(this);
    mWaitCheckBox = new QCheckBox(this);
    mAppendCheckbox = new QCheckBox(this);

    beginGroup("Dialogue Action");
    appendRow(tr("Character"), mChooseCharacterWidget);
    appendRow(tr("Text Box/Dialogue Box"), mChooseTextBoxWidget);
    appendRow(tr("Phrase"), mTextEdit);
    appendRow(tr("Wait on Finished"), mWaitCheckBox);
    appendRow(tr("Append"), mAppendCheckbox);
    endGroup();

    mTextEdit->setMaximumHeight(mTextEdit->height()/2);

    connect(mTextEdit, SIGNAL(textChanged()), this, SLOT(onTextEdited()));
    connect(mChooseTextBoxWidget, SIGNAL(objectChanged(Object*)), this, SLOT(onTextBoxChanged(Object*)));
    connect(mChooseTextBoxWidget, SIGNAL(objectHighlighted(Object*)), this, SLOT(onTextBoxHighlighted(Object*)));
    connect(mChooseCharacterWidget, SIGNAL(objectChanged(Object*)), this, SLOT(onCharacterChanged(Object*)));
    connect(mChooseCharacterWidget, SIGNAL(objectHighlighted(Object*)), this, SLOT(onCharacterHighlighted(Object*)));
    connect(mChooseCharacterWidget, SIGNAL(objectChanged(const QString&)), this, SLOT(onCharacterChanged(const QString&)));
    connect(mWaitCheckBox, SIGNAL(clicked(bool)), this, SLOT(onWaitOnFinishedChanged(bool)));
    connect(mAppendCheckbox, SIGNAL(toggled(bool)), this, SLOT(appendToggled(bool)));

    if (mChooseTextBoxWidget->view())
        mChooseTextBoxWidget->view()->installEventFilter(this);
    if (mChooseCharacterWidget->view())
        mChooseCharacterWidget->view()->installEventFilter(this);

    this->resizeColumnToContents(0);
}


void DialogueEditorWidget::updateData(GameObject* action)
{   
    ActionEditorWidget::updateData(action);

    Dialogue* dialogue = qobject_cast<Dialogue*> (action);
    if (! dialogue)
        return;

    mChooseCharacterWidget->clearEditText();
    mChooseCharacterWidget->loadFromScene(dialogue->scene());
    mChooseCharacterWidget->setEditText(dialogue->characterName());
    mChooseTextBoxWidget->loadFromAction(dialogue);

    if (! mChooseTextBoxWidget->count())
        mTextEdit->setEnabled(false);
    else
        mTextEdit->setEnabled(true);

    mTextEdit->setText(dialogue->text());
    mWaitCheckBox->setChecked(dialogue->mouseClickOnFinish());
    mAppendCheckbox->setChecked(dialogue->append());
}

void DialogueEditorWidget::setGameObject(GameObject* action)
{
    ActionEditorWidget::setGameObject(action);
    setTextInOutputBox();
}

void DialogueEditorWidget::onTextEdited()
{
    Dialogue* dialogue = qobject_cast<Dialogue*> (mGameObject);
    if (! mTextEdit || ! dialogue)
        return;

    dialogue->setText(mTextEdit->toPlainText());
}

void DialogueEditorWidget::onTextBoxChanged(Object* object)
{
    Dialogue* dialogue = qobject_cast<Dialogue*> (mGameObject);
    if (dialogue) {
        dialogue->setSceneObject(object);
        //onTextEdited();
    }
}

void DialogueEditorWidget::onCharacterChanged(Object* obj)
{
    Dialogue* dialogue = qobject_cast<Dialogue*> (mGameObject);
    Character* character = qobject_cast<Character*>(obj);
    if (dialogue && character) {
        dialogue->setCharacter(character);
    }
}

void DialogueEditorWidget::onCharacterChanged(const QString& name)
{
    Dialogue* dialogue = qobject_cast<Dialogue*> (mGameObject);
    if (dialogue) {
        dialogue->setCharacterName(name);
    }
}

void DialogueEditorWidget::onTextBoxHighlighted(Object* obj)
{
    if (mGameObject && mGameObject->scene()) {
        mGameObject->scene()->highlightObject(obj);
    }
}

void DialogueEditorWidget::onCharacterHighlighted(Object* obj)
{
    if (mGameObject && mGameObject->scene() && obj) {
        mGameObject->scene()->highlightObject(obj);
    }
}


bool DialogueEditorWidget::eventFilter(QObject *obj, QEvent *event)
{
    Scene* scene = 0;
    Object* object = 0;
    if (mGameObject)
        scene = mGameObject->scene();

    if ((obj == mChooseTextBoxWidget->view() || obj == mChooseCharacterWidget->view()) && event->type() == QEvent::Hide) {
       if ( scene)
           scene->highlightObject(0);
       return false;
    }

    if (obj == mChooseTextBoxWidget->view() && event->type() == QEvent::Show && scene) {
        object = mChooseTextBoxWidget->objectAt(0);
        if (object)
            scene->highlightObject(object);
    }
    else if (obj == mChooseCharacterWidget->view() && event->type() == QEvent::Show && scene) {
        object = mChooseCharacterWidget->objectAt(0);
        if (object)
            scene->highlightObject(object);
    }


   return false;
}

void DialogueEditorWidget::setTextInOutputBox()
{
    Dialogue* dialogue = qobject_cast<Dialogue*> (mGameObject);
    if (! dialogue || ! dialogue->sceneObject())
        return;

    QString text = dialogue->text();
    DialogueBox * dialogueBox = qobject_cast<DialogueBox*>(dialogue->sceneObject());
    if (dialogueBox) {
        dialogueBox->setText(dialogue->characterName(), text);
    }
    else {
        TextBox* textBox = qobject_cast<TextBox*>(dialogue->sceneObject());
        if (textBox) {
            textBox->setPlaceholderText(text);
        }
    }
}

void DialogueEditorWidget::onWaitOnFinishedChanged(bool state)
{
    Dialogue* dialogue = qobject_cast<Dialogue*> (mGameObject);
    if (dialogue)
        dialogue->setMouseClickOnFinish(state);
}

void DialogueEditorWidget::appendToggled(bool append)
{
    Dialogue* dialogue = qobject_cast<Dialogue*> (mGameObject);
    if (dialogue)
        dialogue->setAppend(append);
}
