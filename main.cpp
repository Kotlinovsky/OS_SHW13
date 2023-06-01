#include <iostream>
#include <fcntl.h>
#include <csignal>
#include <limits>

int main() {
    char temp_dir_template[] = "/tmp/tmpdir.XXXXXX";
    char* temp_dir = mkdtemp(temp_dir_template);

    // Обработаем случай, если временная папка не создалась.
    if (temp_dir == nullptr) {
        std::cerr << "Error during temp dir creating!";
        return 1;
    }

    std::cout << "Created temp dir for experiment: " << temp_dir << std::endl;

    // Начинаем эксперимент, создадим корневой файл.
    std::string temp_dir_path = std::string(temp_dir);
    std::string root_file_path = temp_dir_path + "/0.file";
    int root_fd = open(root_file_path.data(), O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (root_fd == -1) {
        unlink(temp_dir);
        std::cout << "Error during root file creating";
        return 0;
    }

    // Закроем файловый дескриптор к корневому файлу.
    // Необходимо закрывать все файловые дескрипторы, т.к.
    // в таком эксперименте легко наткнуться на их лимит в системе.
    close(root_fd);

    // Теперь попробуем создавать симлинки ...
    for (int i = 1; i < std::numeric_limits<uint64_t>::max(); ++i) {
        std::string from =  temp_dir_path + "/" + std::to_string(i - 1) + ".file";
        std::string to =  temp_dir_path + "/" + std::to_string(i) + ".file";

        if (symlink(from.data(), to.data()) == -1) {
            std::cerr << "Error during symlink creating. Recursion depth = " << i << std::endl;
            break;
        }

        int fd = open(to.data(), O_RDONLY);
        if (fd == -1) {
            std::cerr << "Error during file opening. Recursion depth = " << i << std::endl;
            break;
        }

        // Закроем файловый дескриптор к файлу.
        // Необходимо закрывать все файловые дескрипторы, т.к.
        // в таком эксперименте легко наткнуться на их лимит в системе.
        close(root_fd);
    }

    unlink(temp_dir);
    return 0;
}
