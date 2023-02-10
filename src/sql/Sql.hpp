#pragma once

#include "../pch.hpp"
#include "SqlFunctions.hpp"

class SQL {
public:
    [[nodiscard]] SQL();

    void ExecuteRequest(std::unique_ptr<RequestExecutor> req);

  private:
    pqxx::connection _con;

};
