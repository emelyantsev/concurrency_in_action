#include <iostream>
#include <expected>
#include <string>
#include <charconv>
#include <typeinfo>


std::expected<int, std::string> parseId(std::string_view str) {

    int id;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), id);
    if ( ec != std::errc() ) return std::unexpected("Invalid ID format");
    return id;
}

std::expected<double, std::string> fetchRating(int id) {

    if (id == 42) return 95.5;
    return std::unexpected("User not found in DB");
}

int main() {

    std::string input = "42";

    auto result = parseId( input )
        .and_then(fetchRating)               // Выполнится только если parseId вернул значение
        .transform( [](double r) {            // Преобразуем результат (например, в проценты)
            return r / 100.0; 
        } )
        .or_else( [](std::string err) {      // Если на любом этапе возникла ошибка — логируем
            std::cerr << "Log error: " << err << std::endl;
            return std::expected<double, std::string>( std::unexpected(err) );
        });

    std::cout << typeid(result).name() << std::endl;

    if ( result ) {
        std::cout << "Final score: " << *result << std::endl;
    }
}