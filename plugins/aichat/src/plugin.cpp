// SPDX-FileCopyrightText: 2026 Spotlight Contributors
// SPDX-License-Identifier: MIT

#include "plugin.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFormLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QSettings>
#include <QUuid>
#include <albert/app.h>
#include <albert/icon.h>
#include <albert/logging.h>
#include <albert/notification.h>
#include <albert/standarditem.h>
#include <albert/standardrichitem.h>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

namespace {

const auto k_api_key = u"api_key"_s;
const auto k_api_provider = u"api_provider"_s;
const auto k_model = u"model"_s;

const auto PROVIDER_ANTHROPIC = u"Anthropic (Claude)"_s;
const auto PROVIDER_OPENAI = u"OpenAI"_s;

QString conversationsFilePath(const PluginInstance &p)
{
    return QString::fromStdString(
        (p.dataLocation() / "conversations.json").string());
}

} // namespace

Plugin::Plugin()
{
    loadConversations();
}

Plugin::~Plugin()
{
    saveConversations();
}

QString Plugin::defaultTrigger() const { return u"ai "_s; }

QString Plugin::synopsis(const QString &) const { return u"<message>"_s; }

QString Plugin::apiKey() const
{
    return settings()->value(k_api_key).toString();
}

void Plugin::setApiKey(const QString &key)
{
    settings()->setValue(k_api_key, key);
}

QString Plugin::apiProvider() const
{
    return settings()->value(k_api_provider, PROVIDER_ANTHROPIC).toString();
}

void Plugin::setApiProvider(const QString &provider)
{
    settings()->setValue(k_api_provider, provider);
}

QString Plugin::model() const
{
    auto provider = apiProvider();
    auto default_model = (provider == PROVIDER_ANTHROPIC)
                             ? u"claude-sonnet-4-6"_s
                             : u"gpt-4o"_s;
    return settings()->value(k_model, default_model).toString();
}

void Plugin::setModel(const QString &m)
{
    settings()->setValue(k_model, m);
}

vector<RankItem> Plugin::rankItems(QueryContext &context)
{
    auto query = context.query();
    if (query.trimmed().isEmpty())
        return {};

    auto key = apiKey();
    if (key.isEmpty())
    {
        auto item = StandardItem::make(
            u"ai_nokey"_s,
            u"AI Chat: API key not configured"_s,
            u"Open settings to configure your API key"_s,
            []{ return Icon::grapheme(u"⚠️"_s); },
            vector<Action>{{
                u"settings"_s,
                u"Open AI Chat settings"_s,
                [this]{ App::instance().showSettings(id()); }
            }}
        );
        vector<RankItem> result;
        result.emplace_back(move(item), 1.0);
        return result;
    }

    // Create the "ask AI" action item
    auto ask_item = StandardRichItem::make(
        u"ai_ask"_s,
        u"Ask AI: %1"_s.arg(query),
        u"Send to %1 (%2)"_s.arg(apiProvider(), model()),
        []{ return Icon::grapheme(u"💬"_s); },
        vector<Action>{{
            u"send"_s,
            u"Send message"_s,
            [this, query]() {
                // Start a new conversation or continue current
                if (!current_conversation_)
                {
                    conversations_.push_back({
                        QUuid::createUuid().toString(QUuid::WithoutBraces),
                        query.left(50),
                        {},
                        QDateTime::currentSecsSinceEpoch()
                    });
                    current_conversation_ = &conversations_.back();
                }

                current_conversation_->messages.push_back({u"user"_s, query});

                // Create a notification for feedback
                auto *n = new Notification(u"AI Chat"_s, u"Sending message..."_s);
                n->send();

                sendRequest(query, nullptr);
            }
        }}
    );

    ask_item->setDetailMarkdown(
        u"## AI Chat\n\nPress **Enter** to send your message to **%1** (%2).\n\n"
        u"**Your message:**\n> %3"_s.arg(apiProvider(), model(), query));

    vector<RankItem> result;
    result.emplace_back(move(ask_item), 1.0);
    return result;
}

vector<shared_ptr<Item>> Plugin::handleEmptyQuery()
{
    vector<shared_ptr<Item>> items;

    // Show recent conversations
    for (auto it = conversations_.rbegin();
         it != conversations_.rend() && items.size() < 5; ++it)
    {
        auto &conv = *it;
        auto lastMsg = conv.messages.empty()
                           ? u"Empty conversation"_s
                           : conv.messages.back().content.left(80);

        auto convId = conv.id;
        auto item = StandardRichItem::make(
            u"conv_%1"_s.arg(conv.id),
            conv.title,
            lastMsg,
            []{ return Icon::grapheme(u"💬"_s); },
            vector<Action>{{
                u"continue"_s,
                u"Continue conversation"_s,
                [this, &conv]{
                    current_conversation_ = &conv;
                }
            }, {
                u"delete"_s,
                u"Delete conversation"_s,
                [this, convId]{
                    auto found = std::find_if(conversations_.begin(), conversations_.end(),
                                              [&](const auto &c){ return c.id == convId; });
                    if (found != conversations_.end())
                    {
                        if (current_conversation_ == &(*found))
                            current_conversation_ = nullptr;
                        conversations_.erase(found);
                        saveConversations();
                    }
                },
                false  // don't hide on activation
            }}
        );

        // Build detail markdown from conversation history
        QString detail;
        for (const auto &msg : conv.messages)
        {
            if (msg.role == u"user"_s)
                detail += u"**You:** %1\n\n"_s.arg(msg.content);
            else
                detail += u"**AI:** %1\n\n"_s.arg(msg.content);
        }
        item->setDetailMarkdown(detail);

        items.push_back(move(item));
    }

    if (items.empty())
    {
        items.push_back(StandardItem::make(
            u"ai_hint"_s,
            u"AI Chat"_s,
            u"Type a message after 'ai ' to chat with AI"_s,
            []{ return Icon::grapheme(u"💬"_s); }
        ));
    }

    return items;
}

void Plugin::sendRequest(const QString &userMessage,
                         shared_ptr<Item> responseItem)
{
    Q_UNUSED(userMessage)

    auto key = apiKey();
    auto provider = apiProvider();
    auto modelName = model();

    QUrl url;
    QJsonObject body;
    QNetworkRequest request;

    if (provider == PROVIDER_ANTHROPIC)
    {
        url = QUrl(u"https://api.anthropic.com/v1/messages"_s);
        request.setUrl(url);
        request.setRawHeader("x-api-key", key.toUtf8());
        request.setRawHeader("anthropic-version", "2023-06-01");
        request.setRawHeader("content-type", "application/json");

        QJsonArray messages;
        if (current_conversation_)
        {
            for (const auto &msg : current_conversation_->messages)
                messages.append(QJsonObject{
                    {u"role"_s, msg.role},
                    {u"content"_s, msg.content}
                });
        }

        body = QJsonObject{
            {u"model"_s, modelName},
            {u"max_tokens"_s, 4096},
            {u"messages"_s, messages},
            {u"stream"_s, true}
        };
    }
    else // OpenAI
    {
        url = QUrl(u"https://api.openai.com/v1/chat/completions"_s);
        request.setUrl(url);
        request.setRawHeader("Authorization",
                             u"Bearer %1"_s.arg(key).toUtf8());
        request.setRawHeader("Content-Type", "application/json");

        QJsonArray messages;
        if (current_conversation_)
        {
            for (const auto &msg : current_conversation_->messages)
                messages.append(QJsonObject{
                    {u"role"_s, msg.role},
                    {u"content"_s, msg.content}
                });
        }

        body = QJsonObject{
            {u"model"_s, modelName},
            {u"messages"_s, messages},
            {u"stream"_s, true}
        };
    }

    auto *reply = network_manager_.post(request,
                                        QJsonDocument(body).toJson(QJsonDocument::Compact));

    // Accumulate streaming response
    auto accumulated = make_shared<QString>();
    auto provider_copy = provider;

    connect(reply, &QNetworkReply::readyRead, this,
            [reply, accumulated, provider_copy]()
    {
        auto data = reply->readAll();
        auto lines = data.split('\n');

        for (const auto &line : lines)
        {
            auto trimmed = line.trimmed();
            if (!trimmed.startsWith("data: ") || trimmed == "data: [DONE]")
                continue;

            auto json = QJsonDocument::fromJson(trimmed.mid(6));
            if (json.isNull())
                continue;

            QString token;
            if (provider_copy == PROVIDER_ANTHROPIC)
            {
                auto type = json[u"type"_s].toString();
                if (type == u"content_block_delta"_s)
                    token = json[u"delta"_s][u"text"_s].toString();
            }
            else // OpenAI
            {
                auto choices = json[u"choices"_s].toArray();
                if (!choices.isEmpty())
                    token = choices[0][u"delta"_s][u"content"_s].toString();
            }

            if (!token.isEmpty())
                accumulated->append(token);
        }
    });

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, accumulated]()
    {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
        {
            auto errMsg = reply->errorString();
            WARN << "AI request failed:" << errMsg;

            auto *n = new Notification(u"AI Chat Error"_s, errMsg);
            n->send();
        }
        else if (current_conversation_ && !accumulated->isEmpty())
        {
            current_conversation_->messages.push_back({u"assistant"_s, *accumulated});
            saveConversations();

            auto *n = new Notification(u"AI Chat"_s, u"Response received"_s);
            n->send();
        }
    });
}

void Plugin::loadConversations()
{
    auto path = conversationsFilePath(*this);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray())
        return;

    conversations_.clear();
    for (const auto &convVal : doc.array())
    {
        auto obj = convVal.toObject();
        Conversation conv;
        conv.id = obj[u"id"_s].toString();
        conv.title = obj[u"title"_s].toString();
        conv.timestamp = obj[u"timestamp"_s].toInteger();

        for (const auto &msgVal : obj[u"messages"_s].toArray())
        {
            auto msgObj = msgVal.toObject();
            conv.messages.push_back({
                msgObj[u"role"_s].toString(),
                msgObj[u"content"_s].toString()
            });
        }

        conversations_.push_back(move(conv));
    }
}

void Plugin::saveConversations()
{
    auto path = conversationsFilePath(*this);
    QDir().mkpath(QFileInfo(path).absolutePath());

    QJsonArray arr;
    for (const auto &conv : conversations_)
    {
        QJsonArray messages;
        for (const auto &msg : conv.messages)
        {
            messages.append(QJsonObject{
                {u"role"_s, msg.role},
                {u"content"_s, msg.content}
            });
        }

        arr.append(QJsonObject{
            {u"id"_s, conv.id},
            {u"title"_s, conv.title},
            {u"timestamp"_s, conv.timestamp},
            {u"messages"_s, messages}
        });
    }

    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    auto *layout = new QFormLayout(w);

    // API Provider
    auto *provider_combo = new QComboBox(w);
    provider_combo->addItems({PROVIDER_ANTHROPIC, PROVIDER_OPENAI});
    provider_combo->setCurrentText(apiProvider());
    connect(provider_combo, &QComboBox::currentTextChanged,
            this, &Plugin::setApiProvider);
    layout->addRow(tr("Provider"), provider_combo);

    // API Key
    auto *key_edit = new QLineEdit(w);
    key_edit->setEchoMode(QLineEdit::Password);
    key_edit->setText(apiKey());
    key_edit->setPlaceholderText(tr("Enter your API key"));
    connect(key_edit, &QLineEdit::textChanged, this, &Plugin::setApiKey);
    layout->addRow(tr("API Key"), key_edit);

    // Model
    auto *model_edit = new QLineEdit(w);
    model_edit->setText(model());
    model_edit->setPlaceholderText(tr("e.g., claude-sonnet-4-6 or gpt-4o"));
    connect(model_edit, &QLineEdit::textChanged, this, &Plugin::setModel);
    layout->addRow(tr("Model"), model_edit);

    // Info
    auto *info = new QLabel(tr("Trigger: <b>ai &lt;message&gt;</b><br>"
                               "Conversations are saved locally."), w);
    info->setWordWrap(true);
    layout->addRow(info);

    return w;
}
