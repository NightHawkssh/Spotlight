// SPDX-FileCopyrightText: 2026 Spotlight Contributors
// SPDX-License-Identifier: MIT

#include "icon.h"
#include "standardrichitem.h"
using namespace albert;
using namespace std;

StandardRichItem::~StandardRichItem() {}

// Item interface

void StandardRichItem::setId(QString id) { id_ = ::move(id); }
void StandardRichItem::setText(QString text) { text_ = ::move(text); }
void StandardRichItem::setSubtext(QString subtext) { subtext_ = ::move(subtext); }
void StandardRichItem::setIconFactory(function<unique_ptr<Icon>()> icon_factory) { icon_factory_ = ::move(icon_factory); }
void StandardRichItem::setActions(vector<Action> actions) { actions_ = ::move(actions); }
void StandardRichItem::setInputActionText(QString t) { input_action_text_ = ::move(t); }

QString StandardRichItem::id() const { return id_; }
QString StandardRichItem::text() const { return text_; }
QString StandardRichItem::subtext() const { return subtext_; }

std::unique_ptr<Icon> StandardRichItem::icon() const
{
    if (icon_factory_)
        if (auto icon = icon_factory_(); icon)
            return icon;
    return {};
}

QString StandardRichItem::inputActionText() const { return input_action_text_.isNull() ? text_ : input_action_text_; }
vector<Action> StandardRichItem::actions() const { return actions_; }

// RichItem interface

void StandardRichItem::setDetailMarkdown(QString markdown) { detail_markdown_ = ::move(markdown); }
void StandardRichItem::setMetadata(vector<MetadataEntry> metadata) { metadata_ = ::move(metadata); }
void StandardRichItem::setDetailImageUrl(QString url) { detail_image_url_ = ::move(url); }
void StandardRichItem::setAccessories(vector<Accessory> accessories) { accessories_ = ::move(accessories); }
void StandardRichItem::setThumbnailFactory(function<unique_ptr<Icon>()> thumbnail_factory) { thumbnail_factory_ = ::move(thumbnail_factory); }

QString StandardRichItem::detailMarkdown() const { return detail_markdown_; }
vector<RichItem::MetadataEntry> StandardRichItem::metadata() const { return metadata_; }
QString StandardRichItem::detailImageUrl() const { return detail_image_url_; }
vector<RichItem::Accessory> StandardRichItem::accessories() const { return accessories_; }

unique_ptr<Icon> StandardRichItem::thumbnail() const
{
    if (thumbnail_factory_)
        if (auto icon = thumbnail_factory_(); icon)
            return icon;
    return nullptr;
}
