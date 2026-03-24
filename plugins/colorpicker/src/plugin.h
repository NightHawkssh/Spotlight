// Copyright (c) 2026 Spotlight Contributors

#pragma once
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    QString defaultTrigger() const override;
    QString synopsis(const QString &) const override;
    std::vector<albert::RankItem> rankItems(albert::QueryContext &) override;

};
