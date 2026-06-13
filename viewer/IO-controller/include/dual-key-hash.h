#ifndef     DUAL_KEY_HASH_H
#define     DUAL_KEY_HASH_H

#include    <QHash>
#include    <QString>
#include    <QDebug>
#include    <memory>
#include    <optional>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <typename Key1, typename Key2, typename Value>
class DualKeyHash
{

private:

    // Внутренняя структура для хранения обоих ключей и значения
    struct Node {
        Key1 k1;
        Key2 k2;
        Value value;
    };

    using NodePtr = std::shared_ptr<Node>;

    QHash<Key1, NodePtr> m_byKey1;
    QHash<Key2, NodePtr> m_byKey2;

public:

    // Вставка возвращает true при успехе, false если один из ключей уже занят
    bool insert(const Key1& k1, const Key2& k2, const Value& v)
    {
        if (m_byKey1.contains(k1) || m_byKey2.contains(k2))
        {
            return false;
        }

        auto node = std::make_shared<Node>(Node{k1, k2, v});
        m_byKey1.insert(k1, node);
        m_byKey2.insert(k2, node);
        return true;
    }

    // Поиск по первому ключу
    std::optional<Value> getByKey1(const Key1& k1) const
    {
        auto it = m_byKey1.constFind(k1);
        if (it != m_byKey1.constEnd())
        {
            return it.value()->value;
        }
        return std::nullopt;
    }

    // Поиск по второму ключу
    std::optional<Value> getByKey2(const Key2& k2) const
    {
        auto it = m_byKey2.constFind(k2);
        if (it != m_byKey2.constEnd())
        {
            return it.value()->value;
        }
        return std::nullopt;
    }

    // Удаление по первому ключу (автоматически чистит оба индекса)
    bool removeByKey1(const Key1& k1)
    {
        auto it = m_byKey1.find(k1);
        if (it != m_byKey1.end())
        {
            m_byKey2.remove(it.value()->k2); // Удаляем из второго индекса
            m_byKey1.erase(it);              // Удаляем из первого
            return true;
        }
        return false;
    }

    // Удаление по второму ключу
    bool removeByKey2(const Key2& k2)
    {
        auto it = m_byKey2.find(k2);
        if (it != m_byKey2.end())
        {
            m_byKey1.remove(it.value()->k1);
            m_byKey2.erase(it);
            return true;
        }
        return false;
    }

    int size() const { return m_byKey1.size(); }
};

#endif
