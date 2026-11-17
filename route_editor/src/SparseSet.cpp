#include "editor/SparseSet.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

SparseSet::SparseSet(std::size_t max_size)
    : ids(max_size)
    , id_to_idx(max_size, -1)
    , size(0)
{
}

void SparseSet::add(std::size_t id)
{
    ids[size] = id;
    id_to_idx[id] = size;
    ++size;
}

void SparseSet::remove(std::size_t id)
{
    std::int64_t& idx = id_to_idx[id];
    const std::int64_t last_idx = size - 1;
    if (idx != last_idx)
    {
        const std::size_t last_id = ids[last_idx];
        ids[idx] = last_id;
        id_to_idx[last_id] = idx;
    }
    idx = -1;
    --size;
}

void SparseSet::remove_if_contains(std::size_t id)
{
    if (contains(id))
    {
        remove(id);
    }
}

bool SparseSet::contains(std::size_t id) const
{
    return id_to_idx[id] != -1;
}

void SparseSet::for_each(std::function<void (std::size_t id)> func)
{
    for (std::size_t id : ids)
    {
        func(id);
    }
}

const std::vector<std::size_t>& SparseSet::get_ids() const
{
    return ids;
}

std::vector<std::size_t> SparseSet::copy_shrinked_ids() const
{
    return std::vector<std::size_t>(ids.begin(), ids.begin() + size);
}

std::size_t SparseSet::get_size() const
{
    return size;
}
