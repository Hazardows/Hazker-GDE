#pragma once

#include "defines.h"

// Holds a handle to a file
typedef struct fileHandle {
    // Opaque handle to internal file handle
    void* handle;
    b8 isValid;
} fileHandle;

typedef enum fileModes {
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2
} fileModes;

/**
 * Checks if a file with the given path exists.
 * @param path The path of the file to be checked.
 * @returns true if the file exists, otherwise false.
 */
HAPI b8 filesystem_exists(const char *path);

/**
 * Attempt to open file located at the given path.
 * @param path The path of the file to be opened.
 * @param mode Mode flags for the file when opened.
 * @param binary Indicates if the file should opened in binary mode.
 * @param handle A pointer to a fileHandle structure wich holds the handle information.
 * @returns true if the file was successfully opened, otherwise false.
 */
HAPI b8 filesystem_open(const char *path, fileModes mode, b8 binary, fileHandle *handle);

/**
 * Closes the provided handle to a file.
 * @param handle A pointer to a fileHandle structure wich holds the handle to be closed.
 */
HAPI void filesystem_close(fileHandle *handle);

/**
 * Reads up to a newline or EOF from the provided handle.
 * @param handle A pointer to a fileHandle structure.
 * @param lineBuf A pointer to a character array wich will be allocated and populated by this method.
 * @returns true on success, false on failure.
 */
HAPI b8 filesystem_read_line(fileHandle *handle, char** lineBuf);

/**
 * Writes text to the provided file, appending a '\n' afterward.S
 * @param handle A pointer to a fileHandle structure.
 * @param text The text to be writen.
 * @returns true on success, false on failure.
 */
HAPI b8 filesystem_write_line(fileHandle *handle, const char* text);

/**
 * Reads up to dataSize bytes of data into out_bytes_read.
 * Allocates *out_data, wich must be freed by the caller.
 * @param handle A pointer to a fileHandle structure.
 * @param dataSize The number of bytes to read.
 * @param out_data A pointer to a block of memory to be populated by this method.
 * @param out_bytes_read A pointer to a number wich will be populated with the number of bytes actually read from the file.
 * @returns true on success, false on failure.
 */
HAPI b8 filesystem_read(fileHandle *handle, u64 dataSize, void* out_data, u64* out_bytes_read);

/**
 * Reads up all bytes of data into out_bytes_read.
 * @param handle A pointer to a fileHandle structure.
 * @param out_bytes A pointer to a byte array wich will be allocated and populated by this method.
 * @param out_bytes_read A pointer to a number wich will be populated with the number of bytes actually read from the file.
 * @returns true on success, false on failure.
 */
HAPI b8 filesystem_read_all_bytes(fileHandle *handle, u8** out_bytes, u64* out_bytes_read);

/**
 * Writes provided data to the file.
 * @param handle A pointer to a fileHandle structure.
 * @param dataSize The size of the data in bytes.
 * @param data The data to be written.
 * @param out_bytes_writen A pointer to a number which will be populated with the number of bytes actually written to the file.
 * @returns true on success, false on failure.
 */
HAPI b8 filesystem_write(fileHandle* handle, u64 dataSize, const void* data, u64* out_bytes_writen);
