#ifndef MENUEDITORWIDGET_H
#define MENUEDITORWIDGET_H

#include <QTextEdit>

#include "objectgroup_editor_widget.h"
#include "object_combobox.h"
#include "menu.h"
#include "combobox.h"
#include "actionmanagerbutton.h"

class ObjectComboBox;
class Menu;
class ComboBox;
class ActionManagerButton;

class MenuEditorWidget : public ObjectGroupEditorWidget
{
    Q_OBJECT
public:
    explicit MenuEditorWidget(QWidget *parent = 0);

protected:
    virtual void updateData(GameObject*);
    virtual bool eventFilter(QObject *obj, QEvent *ev);

signals:

private slots:
    void onTextEdited(const QString&);
    void onNumberOfOptionsChanged(int);
    void onButtonChanged(Object*);

private:
    void setNumberOfOptions(int);
    void _updateTexts(Menu*);

private:
    ObjectComboBox* mButtonComboBox;
    QComboBox* mChooseNumberOfOptions;
    int mFirstOptionIndex;
    QList<QLineEdit*> mTextEdits;
    QList<ActionManagerButton*> mActionButtons;
    QList<QTextEdit*> mConditionEdits;
};

#endif // MENUEDITORWIDGET_H
