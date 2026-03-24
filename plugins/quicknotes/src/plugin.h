// Copyright (c) 2026 Spotlight Contributors

#pragma once
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <QDateTime>
#include <QJsonArray>

struct NoteEntry
{
    QString id;
    QString text;
    QDateTime created;
};

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin();

    QString defaultTrigger() const override;
    QString synopsis(const QString &) const override;
    std::vector<albert::RankItem> rankItems(albert::QueryContext &) override;
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery() override;

private:

    void loadNotes();
    void saveNotes();
    void addNote(const QString &text);
    void removeNote(const QString &id);
    std::vector<std::shared_ptr<albert::Item>> noteItems() const;

    std::vector<NoteEntry> notes_;

};
