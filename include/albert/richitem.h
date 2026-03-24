// SPDX-FileCopyrightText: 2026 Spotlight Contributors
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include <albert/export.h>
#include <albert/item.h>
#include <memory>
#include <vector>

namespace albert
{
class Icon;

///
/// Rich item extending \ref Item with detail panel, accessories, and grid support.
///
/// Plugins that want to provide rich content (detail panel, right-side accessories,
/// grid thumbnails) should subclass this instead of Item. Existing plugins using
/// plain Item/StandardItem are unaffected — the frontend checks via dynamic_cast.
///
/// \ingroup core_query
///
class ALBERT_EXPORT RichItem : public Item
{
public:

    ~RichItem() override;

    /// @name Detail Panel
    /// @{

    ///
    /// Returns markdown content for the detail panel.
    ///
    /// When non-empty, the frontend shows a right-side detail panel with this
    /// content rendered as CommonMark markdown. Return empty string to hide the panel.
    ///
    virtual QString detailMarkdown() const;

    ///
    /// Structured metadata entry for the detail panel.
    ///
    struct MetadataEntry
    {
        QString label;   ///< The metadata label (e.g., "Size", "Type").
        QString value;   ///< The metadata value (e.g., "4.2 MB", "PDF").
        QString icon;    ///< Optional icon URL for the label.
    };

    ///
    /// Returns structured metadata shown below the markdown in the detail panel.
    ///
    virtual std::vector<MetadataEntry> metadata() const;

    ///
    /// Returns an image URL or path for the detail panel header.
    ///
    /// Supports local file paths and theme icon URLs.
    ///
    virtual QString detailImageUrl() const;

    /// @}

    /// @name List Accessories
    /// @{

    ///
    /// Accessory displayed on the right side of a list item.
    ///
    struct Accessory
    {
        /// The type of accessory.
        enum Type { Text, Tag, Date, Icon };

        Type type;          ///< Accessory type.
        QString value;      ///< Display value (text, date string, or icon URL).
        QString tooltip;    ///< Optional tooltip shown on hover.
        QString color;      ///< Optional color (hex string), used for Tag type.
    };

    ///
    /// Returns accessories shown on the right side of this item in list view.
    ///
    virtual std::vector<Accessory> accessories() const;

    /// @}

    /// @name Grid View
    /// @{

    ///
    /// Returns a thumbnail icon for grid view.
    ///
    /// When the query handler requests grid view mode, this icon is displayed
    /// as the main visual element. Return nullptr to use the regular icon().
    ///
    virtual std::unique_ptr<Icon> thumbnail() const;

    /// @}
};

}  // namespace albert
