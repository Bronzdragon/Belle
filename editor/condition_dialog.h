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

#ifndef CONDITION_DIALOG_H
#define CONDITION_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QRadioButton>

#include "complexcondition.h"
#include "variablevalidator.h"

#define EDIT_BG_COLOR "#FFE51A"

class ConditionWidget;
class ComplexCondition;
class VariableValidator;

class ConditionDialog : public QDialog
{
    Q_OBJECT

    enum LineEditType {
        Variable = ConditionTokenMetaType::Variable,
        Value = ConditionTokenMetaType::Value
    };

public:
    explicit ConditionDialog(ComplexCondition*, QWidget *parent = 0);

protected:
    virtual void showEvent(QShowEvent *);

private:
    void setLineEditType(QLineEdit*, LineEditType);
    void setDataType(QComboBox*, ConditionTokenMetaType::Type);
    void setCurrentOperation(ConditionOperation::Type);
    void setCurrentLogicalOperator(int);
    void setCurrentLogicalOperator(ConditionLogicalOperator::Type);
    void setEditMode(bool);
    bool editMode() const;
    AbstractCondition* editingCondition() const;
    ConditionToken* editingLogicalOperator() const;
    void reset();
    
signals:
    
private slots:
    void onAddClicked();
    void onCurrentOperatorChanged(int);
    void onLeftMemberEdited(const QString&);
    void onRightMemberEdited(const QString&);
    void onTypeChanged(int);
    void setEditingCondition(AbstractCondition*);

private:
    ConditionWidget* mConditionWidget;
    QLineEdit* mLeftMemberEdit;
    QComboBox* mOperationsComboBox;
    QLineEdit* mRightMemberEdit;
    QComboBox* mComboValueType;
    QRadioButton * mAndRadioButton;
    QRadioButton * mOrRadioButton;
    QComboBox * mLogicalOperators;
    QPushButton* mAddButton;
    QPushButton* mOkButton;
    QComboBox* mDataType1Chooser;
    QComboBox* mDataType2Chooser;
    VariableValidator* mVariableValidator;
    ComplexCondition* mCondition;
    SimpleCondition* mEditingCondition;
    bool mEditMode;
    QIcon mAddIcon;
};

#endif // Condition_DIALOG_H
