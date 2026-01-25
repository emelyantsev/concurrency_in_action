#include <iostream>
#include <expected>
#include <string>
#include <thread>
#include <vector>
#include <chrono>

#include "common.h"


std::expected<int, std::string> validateId( int id ) {

    if ( id <= 0 ) return std::unexpected( "ID должен быть положительным" );
    
    return id;
}

std::expected<std::string, std::string> getDbData( int id ) {

    if ( id == 404 ) return std::unexpected( "Запись не найдена в БД" );
    
    return "User_Data_" + std::to_string( id );
}

void worker( int taskId ) {

    auto result = validateId( taskId )
        .and_then( getDbData )
        .transform( []( std::string data ) {
            return "[Processed] " + data;
        });

    if (result) {
        sync_cout << "Поток " << std::this_thread::get_id() 
                  << " успешно обработал: " << *result << std::endl;
    } else {
        sync_cout << "Поток " << std::this_thread::get_id() 
                  << " завершился с ошибкой: " << result.error() << std::endl;
    }
}

int main() {

    std::vector<int> tasks = {10, -1, 404, 99};
    std::vector<std::jthread> workers;

    sync_cout << "Запуск конвейера обработки..." << std::endl;

    for (int id : tasks) {
        workers.emplace_back( worker, id );
    }

    return 0;
}