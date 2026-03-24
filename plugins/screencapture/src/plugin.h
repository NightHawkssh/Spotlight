// Copyright (c) 2026 Spotlight Contributors

#pragma once
#include <albert/extensionplugin.h>
#include <albert/generatorqueryhandler.h>

class Plugin : public albert::ExtensionPlugin,
               public albert::GeneratorQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();

    QString defaultTrigger() const override;
    QString synopsis(const QString &) const override;
    albert::ItemGenerator items(albert::QueryContext &) override;

private:

    QString screenshotTool_;

};
