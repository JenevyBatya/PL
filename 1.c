#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Структура для хранения пикселя
struct pixel {
    uint8_t b, g, r;
};

// Определения структур BMPInfo
struct  __attribute__((packed)) BMPInfo {
        uint16_t signature;   // Сигнатура файла ('B' 'M')
    uint32_t fileSize;    // Размер файла
    uint32_t reserved;    // Зарезервированное значение (обычно 0)
    uint32_t dataOffset;  // Смещение данных в байтах
    uint32_t infoSize;      // Размер структуры Info
    uint32_t width;         // Ширина изображения
    uint32_t height;        // Высота изображения
    uint16_t planes;        // Количество плоскостей (должно быть 1)
    uint16_t bitCount;      // Количество бит на пиксель (24 для true color)
    uint32_t compression;   // Тип сжатия (обычно 0 для несжатых изображений)
    uint32_t imageSize;     // Размер изображения в байтах
    int32_t xPixelsPerM;  // Горизонтальное разрешение в пикселях на метр
    int32_t yPixelsPerM;  // Вертикальное разрешение в пикселях на метр
    uint32_t colorsUsed;    // Количество цветов в палитре (0 для true color)
    uint32_t colorsImportant; // Количество "важных" цветов (0 если все цвета важны)
};

struct image {
    uint64_t width, height;
    struct pixel* data;
};
uint64_t calculate_padding(uint64_t width){
    uint64_t bytes_in_row = width* sizeof(struct pixel);
    return (4 - (bytes_in_row % 4))%4;
}








//Компилится хорошо, пока нареканий нет
// Функция чтения файла
int read_BMP_file(const char* input_file_path, struct BMPInfo* info, struct image* image) {
    //Открываем файл
    FILE* file = fopen(input_file_path, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }
//Считываем основную информацию по файлу
    fread(info, sizeof(struct BMPInfo), 1, file);
    image->width = info->width;
    image->height = info->height;
    image->data = malloc(image->height * image->width * sizeof(struct pixel));

//Считываем пиксели
    uint64_t padding = calculate_padding(image->width);

    for (size_t i = 0; i < image->height; i++) {
        fread( &(image->data[i*image->width]), sizeof(struct pixel), image->width, file);
        fseek(file, (long) padding, SEEK_CUR);
}
    fclose(file);
    return 0;
}


//Компилится хорошо, пока нареканий нет(скорее всего, ошибка где-то здесь)
//Функция поворота картинки, принимает заполненный объект первоначальной картинки и пустой для новой картинки
int rotate(struct image* image, struct image* imageOut, struct BMPInfo* info, struct BMPInfo* infoForNew) {
//меняем местами ширину и высоту, создаем в стеке новое пространство для смены пикселей
    imageOut->width = image->height;
    imageOut->height = image->width;
    imageOut->data = (struct pixel*)malloc(imageOut->height * imageOut->width * sizeof(struct pixel));
    if(!imageOut->data) return 1;

    for (size_t j = 0; j < imageOut->height; j++) {
        for (size_t i = 0; i < imageOut->width; i++) {
            imageOut->data[j * imageOut->width + i] = image->data[(image->height - i - 1) * image->width + j];
        }
    }
    *infoForNew = *info;
    infoForNew->height = imageOut->height;
    infoForNew->width = imageOut->width;

    return 0;
}




//Запись в файл
int write_data_in_file(const char* output_file_path, struct BMPInfo* info, struct image* image) {
    FILE* output_file = fopen(output_file_path, "wb");
    if (output_file == NULL) {
        perror("Failed to open output file");
        return 1;
    }
    uint64_t padding = calculate_padding(image->width);
    if(!fwrite(info, sizeof(struct BMPInfo), 1, output_file)){
        return 1;
    }

    for (size_t j = 0; j < image->height; j++){
        if(!fwrite(&image->data[j*image->width], image->width*sizeof(struct pixel), 1, output_file)){
            return 1;
        }

        for(size_t i = 0; i < padding; i++){
            fputc(0, output_file);
        }
    }

    fclose(output_file);
    return 0;
}


int main(){
    struct BMPInfo info;
    struct image image;
    struct image imageOut;
    struct BMPInfo infoForNew;
    read_BMP_file("input.bmp", &info, &image);
    int width = info.width;
    int height = info.height;
    // infoForNew = info;
    // infoForNew.height = info.width;
    // infoForNew.width = info.height;
    // Выводим информацию
    printf("Исходное изображениеsdv:\n");
    printf("width: %d pixels\n", image.width);
    printf("height: %d pixels\n", image.height);
    rotate(&image, &imageOut, &info, &infoForNew);

    write_data_in_file("output.bmp", &info, &image);
    return 0;



    
}