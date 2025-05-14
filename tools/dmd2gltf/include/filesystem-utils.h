#ifndef     FILESYSTEM_UTILS_H
#define     FILESYSTEM_UTILS_H

#include    <string>

/// Кроссплатфоременный разделитель в пути
char separator();

/// Приведение пути к необходимому платформе виду
void path_to_native_separator(std::string &path);

/// Добавление к пути подпути
std::string combine_path(const std::string &path, const std::string &subpath);

#endif // FILESYSTEM_UTILS_H
