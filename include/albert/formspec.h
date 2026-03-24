// SPDX-FileCopyrightText: 2026 Spotlight Contributors
// SPDX-License-Identifier: MIT

#pragma once
#include <QStringList>
#include <QVariantMap>
#include <albert/export.h>
#include <functional>
#include <vector>

namespace albert
{

///
/// Describes a single field in a form.
///
/// \ingroup core_query
///
struct ALBERT_EXPORT FormField
{
    /// The type of form field.
    enum Type {
        TextField,
        TextArea,
        Dropdown,
        Checkbox,
        DatePicker,
        FilePicker,
        TagPicker
    };

    Type type = TextField;          ///< Field type.
    QString id;                     ///< Unique identifier for the field value.
    QString label;                  ///< Display label.
    QString placeholder;            ///< Placeholder text.
    QString defaultValue;           ///< Default value.
    QStringList options;            ///< Options for Dropdown and TagPicker types.
    bool required = false;          ///< Whether the field must be filled.
};

///
/// Describes a complete form with fields and a submission callback.
///
/// \ingroup core_query
///
struct ALBERT_EXPORT FormSpec
{
    QString title;                             ///< Form title displayed at the top.
    std::vector<FormField> fields;             ///< Form fields.
    std::function<void(QVariantMap)> onSubmit;  ///< Called with field id → value map on submit.
};

}  // namespace albert
