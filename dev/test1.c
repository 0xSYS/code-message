
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lodepng.h"

void encode_message(unsigned char *image, unsigned width, unsigned height, const char *message) {
    size_t message_length = strlen(message);
    size_t max_message_size = width * height * 4 / 8;

    if (message_length + 1 > max_message_size) {
        fprintf(stderr, "Message too long to fit in the image.\n");
        exit(1);
    }

    // Write message length and message to the image
    size_t index = 0;
    for (size_t i = 0; i < sizeof(size_t); i++) {
        image[index++] = (message_length >> (i * 8)) & 0xFF;
    }

    for (size_t i = 0; i <= message_length; i++) {
        for (int bit = 0; bit < 8; bit++) {
            if (index >= width * height * 4) {
                fprintf(stderr, "Unexpected end of image while encoding message.\n");
                exit(1);
            }
            image[index] = (image[index] & ~1) | ((message[i] >> bit) & 1);
            index++;
        }
    }
}

char *decode_message(const unsigned char *image, unsigned width, unsigned height) {
    size_t index = 0;

    // Read message length
    size_t message_length = 0;
    for (size_t i = 0; i < sizeof(size_t); i++) {
        message_length |= ((size_t)image[index++]) << (i * 8);
    }

    if (message_length > width * height * 4 / 8) {
        fprintf(stderr, "Message size exceeds capacity of the image.\n");
        exit(1);
    }

    // Read the message
    char *message = malloc(message_length + 1);
    if (!message) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(1);
    }

    for (size_t i = 0; i <= message_length; i++) {
        message[i] = 0;
        for (int bit = 0; bit < 8; bit++) {
            if (index >= width * height * 4) {
                fprintf(stderr, "Unexpected end of image while decoding message.\n");
                exit(1);
            }
            message[i] |= (image[index++] & 1) << bit;
        }
    }

    return message;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <encode/decode> <input.png> <output.png/message> <message>\n", argv[0]);
        return 1;
    }

    const char *action = argv[1];
    const char *input_file = argv[2];
    const char *output_file_or_message = argv[3];

    unsigned char *image;
    unsigned width, height;

    // Load the PNG
    if (lodepng_decode32_file(&image, &width, &height, input_file)) {
        fprintf(stderr, "Error loading PNG file.\n");
        return 1;
    }

    if (strcmp(action, "encode") == 0) {
        const char *message = argv[4];
        encode_message(image, width, height, message);
        if (lodepng_encode32_file(output_file_or_message, image, width, height)) {
            fprintf(stderr, "Error saving PNG file.\n");
            return 1;
        }
        printf("Message encoded successfully.\n");
    } else if (strcmp(action, "decode") == 0) {
        char *message = decode_message(image, width, height);
        printf("Decoded message: %s\n", message);
        free(message);
    } else {
        fprintf(stderr, "Unknown action: %s\n", action);
        return 1;
    }

    free(image);
    return 0;
}
