#ifndef EDITOR_SPARSE_SET_H
#define EDITOR_SPARSE_SET_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

class SparseSet
{
public:
    explicit SparseSet(std::size_t max_size);

    void add(std::size_t id);
    void remove(std::size_t id);
    void remove_if_contains(std::size_t id);

    bool contains(std::size_t id) const;

    void for_each(std::function<void (std::size_t id)> func);

    const std::vector<std::size_t>& get_ids() const;
    std::vector<std::size_t> copy_shrinked_ids() const;

    std::size_t get_size() const;

private:
    std::vector<std::size_t> ids;
    std::vector<std::int64_t> id_to_idx;
    std::size_t size;
};

#endif // EDITOR_SPARSE_SET_H
