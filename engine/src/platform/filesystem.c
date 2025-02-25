#include "platform/filesystem.h"

#include "core/logger.h"
#include "memory/hmemory.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

b8 filesystem_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

b8 filesystem_open(const char* path, fileModes mode, b8 binary, fileHandle* out_handle) {
    out_handle->isValid = false;
    out_handle->handle = 0;
    const char* mode_str;

    if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
        mode_str = binary ? "w+b" : "w+";
    } 
    else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
        mode_str = binary ? "rb" : "r";
    }
    else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
        mode_str = binary ? "wb" : "w";
    } 
    else {
        HERROR("Invalid mode passed while trying to open file: '%s'", path);
        return false;
    }

    // Attempt to open the file.
    FILE* file = fopen(path, mode_str);
    if (!file) {
        HERROR("Error opening file: '%s'", path);
        return false;
    }

    out_handle->handle = file;
    out_handle->isValid = true;

    return true;
}

void filesystem_close(fileHandle* handle) {
    if (handle->handle) {
        fclose((FILE*)handle->handle);
        handle->handle = 0;
        handle->isValid = false;
    }
}

b8 filesystem_read_line(fileHandle* handle, char** line_buf) {
    if (handle->handle) {
        // Since we are reading a single line, it should be safe to assume this is enough characters.
        char buffer[32000];
        if (fgets(buffer, 32000, (FILE*)handle->handle) != 0) {
            u64 length = strlen(buffer);
            *line_buf = Hallocate((sizeof(char) * length) + 1, MEMORY_TAG_STRING);
            strcpy(*line_buf, buffer);
            return true;
        }
    }
    return false;
}

b8 filesystem_write_line(fileHandle* handle, const char* text) {
    if (handle->handle) {
        i32 result = fputs(text, (FILE*)handle->handle);
        if (result != EOF) {
            result = fputc('\n', (FILE*)handle->handle);
        }

        // Make sure to flush the stream so it is written to the file immediately.
        // This prevents data loss in the event of a crash.
        fflush((FILE*)handle->handle);
        return result != EOF;
    }
    return false;
}

b8 filesystem_read(fileHandle* handle, u64 data_size, void* out_data, u64* out_bytes_read) {
    if (handle->handle && out_data) {
        *out_bytes_read = fread(out_data, 1, data_size, (FILE*)handle->handle);
        if (*out_bytes_read != data_size) {
            return false;
        }
        return true;
    }
    return false;
}

b8 filesystem_read_all_bytes(fileHandle* handle, u8** out_bytes, u64* out_bytes_read) {
    if (handle->handle) {
        // File size
        fseek((FILE*)handle->handle, 0, SEEK_END);
        u64 size = ftell((FILE*)handle->handle);
        rewind((FILE*)handle->handle);

        *out_bytes = Hallocate(sizeof(u8) * size, MEMORY_TAG_STRING);
        *out_bytes_read = fread(*out_bytes, 1, size, (FILE*)handle->handle);
        if (*out_bytes_read != size) {
            return false;
        }
        return true;
    }
    return false;
}

b8 filesystem_write(fileHandle* handle, u64 data_size, const void* data, u64* out_bytes_written) {
    if (handle->handle) {
        *out_bytes_written = fwrite(data, 1, data_size, (FILE*)handle->handle);
        if (*out_bytes_written != data_size) {
            return false;
        }
        fflush((FILE*)handle->handle);
        return true;
    }
    return false;
}
