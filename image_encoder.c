#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void xorEncryptDecrypt(char *data, const char *key, size_t dataLen) {
    size_t keyLen = strlen(key);
    for (size_t i = 0; i < dataLen; i++) {
        data[i] ^= key[i % keyLen];
    }
}

void encodeMessage(const char *inputImage, const char *outputImage, const char *message, const char *key, const char *messageFile) {
    FILE *in = fopen(inputImage, "rb");
    FILE *out = fopen(outputImage, "wb");
    if (!in || !out) {
        perror("File error");
        if (in) fclose(in);
        if (out) fclose(out);
        return;
    }

    unsigned char header[54];
    fread(header, 1, 54, in);
    fwrite(header, 1, 54, out);

    size_t messageLen = strlen(message);
    char *encryptedMessage = malloc(messageLen);
    if (!encryptedMessage) {
        perror("Memory error");
        fclose(in);
        fclose(out);
        return;
    }
    strcpy(encryptedMessage, message);
    xorEncryptDecrypt(encryptedMessage, key, messageLen);

    // Save the encrypted message to a text file
    FILE *msgFile = fopen(messageFile, "wb");
    if (!msgFile) {
        perror("Message file error");
        free(encryptedMessage);
        fclose(in);
        fclose(out);
        return;
    }
    fwrite(encryptedMessage, 1, messageLen, msgFile);
    fclose(msgFile);

    // Write message length to image
    for (size_t i = 0; i < sizeof(size_t); i++) {
        unsigned char byte = (messageLen >> (i * 8)) & 0xFF;
        fwrite(&byte, 1, 1, out);
    }

    unsigned char buffer;
    for (size_t i = 0; i < messageLen; i++) {
        for (int bit = 0; bit < 8; bit++) {
            fread(&buffer, 1, 1, in);
            buffer = (buffer & 0xFE) | ((encryptedMessage[i] >> bit) & 0x01);
            fwrite(&buffer, 1, 1, out);
        }
    }

    free(encryptedMessage);
    while (fread(&buffer, 1, 1, in)) {
        fwrite(&buffer, 1, 1, out);
    }

    fclose(in);
    fclose(out);
    printf("Message encoded successfully!\n");
}

void decodeMessage(const char *inputImage, const char *key, const char *messageFile) {
    FILE *in = fopen(inputImage, "rb");
    if (!in) {
        perror("File error");
        return;
    }

    fseek(in, 54, SEEK_SET);

    size_t messageLen = 0;
    for (size_t i = 0; i < sizeof(size_t); i++) {
        unsigned char byte;
        fread(&byte, 1, 1, in);
        messageLen |= ((size_t)byte << (i * 8));
    }

    char *encryptedMessage = malloc(messageLen + 1);
    if (!encryptedMessage) {
        perror("Memory error");
        fclose(in);
        return;
    }

    // Read the encoded message from the text file
    FILE *msgFile = fopen(messageFile, "rb");
    if (!msgFile) {
        perror("Message file error");
        free(encryptedMessage);
        fclose(in);
        return;
    }
    fread(encryptedMessage, 1, messageLen, msgFile);
    fclose(msgFile);

    // Decrypt the message
    xorEncryptDecrypt(encryptedMessage, key, messageLen);
    printf("Decoded message: %s\n", encryptedMessage);

    free(encryptedMessage);
    fclose(in);
}

int main() {
    int choice;
    char inputImage[256], outputImage[256], message[256], key[256], messageFile[256];

    printf("1. Encode message\n2. Decode message\nEnter your choice: ");
    scanf("%d", &choice);
    getchar();

    if (choice == 1) {
        printf("Enter input image path: ");
        fgets(inputImage, sizeof(inputImage), stdin);
        inputImage[strcspn(inputImage, "\n")] = '\0';

        printf("Enter output image path: ");
        fgets(outputImage, sizeof(outputImage), stdin);
        outputImage[strcspn(outputImage, "\n")] = '\0';

        printf("Enter message to encode: ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        printf("Enter encryption key: ");
        fgets(key, sizeof(key), stdin);
        key[strcspn(key, "\n")] = '\0';

        printf("Enter path for message file to store encoded message: ");
        fgets(messageFile, sizeof(messageFile), stdin);
        messageFile[strcspn(messageFile, "\n")] = '\0';

        encodeMessage(inputImage, outputImage, message, key, messageFile);
    } else if (choice == 2) {
        printf("Enter encoded image path: ");
        fgets(inputImage, sizeof(inputImage), stdin);
        inputImage[strcspn(inputImage, "\n")] = '\0';

        printf("Enter decryption key: ");
        fgets(key, sizeof(key), stdin);
        key[strcspn(key, "\n")] = '\0';

        printf("Enter path for message file to read encoded message: ");
        fgets(messageFile, sizeof(messageFile), stdin);
        messageFile[strcspn(messageFile, "\n")] = '\0';

        decodeMessage(inputImage, key, messageFile);
    } else {
        printf("Invalid choice.\n");
    }

    return 0;
}
