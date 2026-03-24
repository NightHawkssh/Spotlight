// SPDX-FileCopyrightText: 2026 Spotlight Contributors
// SPDX-License-Identifier: MIT

#pragma once
#include <albert/richitem.h>
#include <memory>
#include <vector>
namespace albert{ class Icon; }

namespace albert
{

///
/// General purpose \ref RichItem implementation.
///
/// Extends \ref StandardItem's pattern with rich content fields (detail markdown,
/// metadata, accessories, thumbnail). All rich fields are optional and default to empty.
///
/// \ingroup util_query
///
class ALBERT_EXPORT StandardRichItem : public RichItem
{
public:

    ///
    /// Constructs a \ref StandardRichItem with the contents initialized with the data passed.
    ///
    template<typename T_ID = QString,
             typename T_TEXT = QString,
             typename T_SUBTEXT = QString,
             typename T_ICON_FACTORY = std::function<std::unique_ptr<Icon>()>,
             typename T_ACTIONS = std::vector<Action>,
             typename T_INPUTACTION = QString>
        requires(std::same_as<std::remove_cvref_t<T_ID>, QString>
                 && std::same_as<std::remove_cvref_t<T_TEXT>, QString>
                 && std::same_as<std::remove_cvref_t<T_SUBTEXT>, QString>
                 && std::convertible_to<std::remove_cvref_t<T_ICON_FACTORY>, std::function<std::unique_ptr<Icon>()>>
                 && std::same_as<std::remove_cvref_t<T_ACTIONS>, std::vector<Action>>
                 && std::same_as<std::remove_cvref_t<T_INPUTACTION>, QString>)
    StandardRichItem(T_ID &&id,
                     T_TEXT &&text,
                     T_SUBTEXT &&subtext,
                     T_ICON_FACTORY &&icon_factory,
                     T_ACTIONS &&actions = std::vector<Action>{},
                     T_INPUTACTION &&input_action_text = QString{}) noexcept :
        id_(std::forward<T_ID>(id)),
        text_(std::forward<T_TEXT>(text)),
        subtext_(std::forward<T_SUBTEXT>(subtext)),
        icon_factory_(std::forward<T_ICON_FACTORY>(icon_factory)),
        actions_(std::forward<T_ACTIONS>(actions)),
        input_action_text_(std::forward<T_INPUTACTION>(input_action_text))
    {}

    ///
    /// Constructs a `shared_ptr` holding a \ref StandardRichItem.
    ///
    template<typename T_ID = QString,
             typename T_TEXT = QString,
             typename T_SUBTEXT = QString,
             typename T_ICON_FACTORY = std::function<std::unique_ptr<Icon>()>,
             typename T_ACTIONS = std::vector<Action>,
             typename T_INPUTACTION = QString>
        requires(std::same_as<std::decay_t<T_ID>, QString>
                 && std::same_as<std::decay_t<T_TEXT>, QString>
                 && std::same_as<std::decay_t<T_SUBTEXT>, QString>
                 && std::convertible_to<std::decay_t<T_ICON_FACTORY>, std::function<std::unique_ptr<Icon>()>>
                 && std::same_as<std::decay_t<T_ACTIONS>, std::vector<Action>>
                 && std::same_as<std::decay_t<T_INPUTACTION>, QString>)
    static std::shared_ptr<StandardRichItem> make(T_ID &&id,
                                                  T_TEXT &&text,
                                                  T_SUBTEXT &&subtext,
                                                  T_ICON_FACTORY &&icon_factory,
                                                  T_ACTIONS &&actions = std::vector<Action>{},
                                                  T_INPUTACTION &&input_action_text = QString{}) noexcept
    {
        return std::make_shared<StandardRichItem>(std::forward<T_ID>(id),
                                                  std::forward<T_TEXT>(text),
                                                  std::forward<T_SUBTEXT>(subtext),
                                                  std::forward<T_ICON_FACTORY>(icon_factory),
                                                  std::forward<T_ACTIONS>(actions),
                                                  std::forward<T_INPUTACTION>(input_action_text));
    }

    StandardRichItem(const StandardRichItem &) = delete;
    StandardRichItem& operator=(const StandardRichItem&) = delete;
    StandardRichItem(StandardRichItem &&other) noexcept = default;
    StandardRichItem &operator=(StandardRichItem &&other) noexcept = default;
    ~StandardRichItem() override;

    // Item interface
    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QString inputActionText() const override;
    std::unique_ptr<Icon> icon() const override;
    std::vector<Action> actions() const override;

    // RichItem interface
    QString detailMarkdown() const override;
    std::vector<MetadataEntry> metadata() const override;
    QString detailImageUrl() const override;
    std::vector<Accessory> accessories() const override;
    std::unique_ptr<Icon> thumbnail() const override;

    /// @name Setters
    /// @{

    void setId(QString id);
    void setText(QString text);
    void setSubtext(QString text);
    void setIconFactory(std::function<std::unique_ptr<Icon>()> icon_factory);
    void setActions(std::vector<Action> actions);
    void setInputActionText(QString text);

    void setDetailMarkdown(QString markdown);
    void setMetadata(std::vector<MetadataEntry> metadata);
    void setDetailImageUrl(QString url);
    void setAccessories(std::vector<Accessory> accessories);
    void setThumbnailFactory(std::function<std::unique_ptr<Icon>()> thumbnail_factory);

    /// @}

protected:
    QString id_;
    QString text_;
    QString subtext_;
    std::function<std::unique_ptr<Icon>()> icon_factory_;
    std::vector<Action> actions_;
    QString input_action_text_;

    // Rich fields
    QString detail_markdown_;
    std::vector<MetadataEntry> metadata_;
    QString detail_image_url_;
    std::vector<Accessory> accessories_;
    std::function<std::unique_ptr<Icon>()> thumbnail_factory_;
};

}  // namespace albert
