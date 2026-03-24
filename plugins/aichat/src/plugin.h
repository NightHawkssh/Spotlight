// SPDX-FileCopyrightText: 2026 Spotlight Contributors
// SPDX-License-Identifier: MIT

#pragma once
#include <QNetworkAccessManager>
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <memory>
#include <vector>

namespace albert { class Item; }

class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();
    ~Plugin() override;

    // QueryHandler interface
    QString defaultTrigger() const override;
    QString synopsis(const QString &) const override;

    // RankedQueryHandler interface
    std::vector<albert::RankItem> rankItems(albert::QueryContext &context) override;

    // GlobalQueryHandler interface
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery() override;

    // PluginInstance interface
    QWidget *buildConfigWidget() override;

private:

    struct Conversation {
        QString id;
        QString title;
        struct Message {
            QString role;    // "user" or "assistant"
            QString content;
        };
        std::vector<Message> messages;
        qint64 timestamp;
    };

    void sendRequest(const QString &userMessage,
                     std::shared_ptr<albert::Item> responseItem);

    void loadConversations();
    void saveConversations();

    QString apiKey() const;
    void setApiKey(const QString &key);

    QString apiProvider() const;
    void setApiProvider(const QString &provider);

    QString model() const;
    void setModel(const QString &model);

    QNetworkAccessManager network_manager_;
    std::vector<Conversation> conversations_;
    Conversation *current_conversation_ = nullptr;
};
