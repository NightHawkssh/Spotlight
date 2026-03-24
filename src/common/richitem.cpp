// SPDX-FileCopyrightText: 2026 Spotlight Contributors
// SPDX-License-Identifier: MIT

#include "icon.h"
#include "richitem.h"
using namespace albert;

RichItem::~RichItem() {}

QString RichItem::detailMarkdown() const { return {}; }

std::vector<RichItem::MetadataEntry> RichItem::metadata() const { return {}; }

QString RichItem::detailImageUrl() const { return {}; }

std::vector<RichItem::Accessory> RichItem::accessories() const { return {}; }

std::unique_ptr<Icon> RichItem::thumbnail() const { return nullptr; }
