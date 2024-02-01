#include <iostream>
#include <chrono>

#include "scheme.h"
#include "error.h"

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    Interpreter interp;
    std::vector<std::string> hist;
    std::time_t cur_time = std::chrono::system_clock::to_time_t(std::chrono::high_resolution_clock::now());
    std::string str_time = std::string(std::ctime(&cur_time));
    str_time.pop_back();
    rl_bind_key ('\t', rl_insert);
    std::cout << "Scheme Interpreter (v1.0.0) [" << str_time << "] on linux" << std::endl;
    while (true) {
        int balance = 0;
        std::string total_cmd = "";
        bool first = true;
        do {
            std::string cmd = readline(first ? ">>> " : "... ");
            add_history(cmd.c_str());
            for (auto &x : cmd) {
                if (x == '(') ++balance;
                else if (x == ')') --balance;
            }
            first = false;
            total_cmd += cmd + " ";
        } while (balance != 0);
        try {
            std::cout << interp.Run(total_cmd) << std::endl;
        } catch (const SyntaxError& err) {
            std::cout << "Syntax error: " << err.what() << std::endl;
        } catch (const RuntimeError& err) {
            std::cout << "Runtime error: " << err.what() << std::endl;
        } catch (const NameError& err) {
            std::cout << "Name error: " << err.what() << std::endl;
        }
    }
    return 0;
}
