// Copyright (c) 2026 Spotlight Contributors

#include "plugin.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QUuid>
#include <albert/icon.h>
#include <albert/logging.h>
#include <albert/standardrichitem.h>
#include <albert/systemutil.h>
ALBERT_LOGGING_CATEGORY("quicknotes")
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

static const auto NOTES_FILE = u"notes.json"_s;
static const auto k_id = u"id"_s;
static const auto k_text = u"text"_s;
static const auto k_created = u"created"_s;

Plugin::Plugin()
{
    loadNotes();
}

Plugin::~Plugin()
{
    saveNotes();
}

void Plugin::loadNotes()
{
    QDir dir(dataLocation());
    QFile file(dir.filePath(NOTES_FILE));
    if (!file.open(QIODevice::ReadOnly))
        return;

    auto arr = QJsonDocument::fromJson(file.readAll()).array();
    file.close();

    for (const auto &val : arr)
    {
        auto obj = val.toObject();
        notes_.push_back({
            obj[k_id].toString(),
            obj[k_text].toString(),
            QDateTime::fromSecsSinceEpoch(obj[k_created].toInteger())
        });
    }

    DEBG << u"Loaded %1 notes"_s.arg(notes_.size());
}

void Plugin::saveNotes()
{
    QDir dir(dataLocation());
    if (!dir.exists())
        dir.mkpath(u"."_s);

    QFile file(dir.filePath(NOTES_FILE));
    if (!file.open(QIODevice::WriteOnly))
    {
        WARN << u"Failed to save notes to"_s << file.fileName();
        return;
    }

    QJsonArray arr;
    for (const auto &note : notes_)
    {
        QJsonObject obj;
        obj[k_id] = note.id;
        obj[k_text] = note.text;
        obj[k_created] = note.created.toSecsSinceEpoch();
        arr.append(obj);
    }

    file.write(QJsonDocument(arr).toJson());
    file.close();
}

void Plugin::addNote(const QString &text)
{
    NoteEntry entry{
        QUuid::createUuid().toString(QUuid::WithoutBraces),
        text,
        QDateTime::currentDateTime()
    };
    notes_.insert(notes_.begin(), entry);
    saveNotes();
}

void Plugin::removeNote(const QString &id)
{
    auto it = find_if(notes_.begin(), notes_.end(),
                       [&id](const auto &n){ return n.id == id; });
    if (it != notes_.end())
    {
        notes_.erase(it);
        saveNotes();
    }
}

vector<shared_ptr<Item>> Plugin::noteItems() const
{
    vector<shared_ptr<Item>> items;
    QLocale loc;

    for (const auto &note : notes_)
    {
        auto preview = note.text;
        if (preview.size() > 80)
            preview = preview.left(80) + u" ..."_s;

        auto detail = u"## Note\n\n%1\n\n---\n\n*Created: %2*\n"_s
            .arg(note.text, loc.toString(note.created, QLocale::LongFormat));

        auto item = StandardRichItem::make(
            note.id,
            preview,
            loc.toString(note.created, QLocale::ShortFormat),
            [] { return Icon::grapheme(u"\U0001f4dd"_s); },
            vector<Action>{
                {
                    u"copy"_s, tr("Copy"),
                    [t = note.text] { setClipboardText(t); }
                },
                {
                    u"del"_s, tr("Delete"),
                    [this, id = note.id] { const_cast<Plugin*>(this)->removeNote(id); }
                }
            }
        );

        item->setDetailMarkdown(detail);
        item->setMetadata({
            {u"Created"_s, loc.toString(note.created, QLocale::LongFormat), {}},
            {u"Length"_s, u"%1 characters"_s.arg(note.text.size()), {}}
        });

        items.push_back(move(item));
    }

    return items;
}

QString Plugin::defaultTrigger() const { return u"note "_s; }

QString Plugin::synopsis(const QString &) const { return u"<text to save>"_s; }

vector<RankItem> Plugin::rankItems(QueryContext &ctx)
{
    vector<RankItem> results;

    auto query = ctx.query().trimmed();

    if (query.isEmpty())
    {
        // Show recent notes
        auto items = noteItems();
        for (auto &item : items)
            results.emplace_back(move(item), 0.0);
        return results;
    }

    // Offer to create a new note
    auto item = StandardRichItem::make(
        u"new"_s,
        tr("Save note: %1").arg(query),
        tr("Create a new quick note"),
        [] { return Icon::grapheme(u"\u2795"_s); },
        vector<Action>{
            {
                u"save"_s, tr("Save note"),
                [this, q = query] { const_cast<Plugin*>(this)->addNote(q); }
            }
        }
    );

    auto detail = u"## New Note\n\n%1\n"_s.arg(query);
    item->setDetailMarkdown(detail);

    results.emplace_back(move(item), 1.0);

    // Also show matching existing notes
    for (const auto &note : notes_)
    {
        if (note.text.contains(query, Qt::CaseInsensitive))
        {
            QLocale loc;
            auto preview = note.text;
            if (preview.size() > 80)
                preview = preview.left(80) + u" ..."_s;

            auto noteDetail = u"## Note\n\n%1\n\n---\n\n*Created: %2*\n"_s
                .arg(note.text, loc.toString(note.created, QLocale::LongFormat));

            auto noteItem = StandardRichItem::make(
                note.id,
                preview,
                loc.toString(note.created, QLocale::ShortFormat),
                [] { return Icon::grapheme(u"\U0001f4dd"_s); },
                vector<Action>{
                    {
                        u"copy"_s, tr("Copy"),
                        [t = note.text] { setClipboardText(t); }
                    },
                    {
                        u"del"_s, tr("Delete"),
                        [this, id = note.id] { const_cast<Plugin*>(this)->removeNote(id); }
                    }
                }
            );

            noteItem->setDetailMarkdown(noteDetail);
            results.emplace_back(move(noteItem), 0.5);
        }
    }

    return results;
}

vector<shared_ptr<Item>> Plugin::handleEmptyQuery()
{
    return noteItems();
}
