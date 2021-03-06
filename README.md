# Лабораторная работа по оптимизации ПО

## Задача

Пусть дана матрица, в которой значения хранятся по столбцам, причём столбцы не лежат на непрерывном участке памяти. Для эффективного использования ресурсов в дальнейшем, например для перемножения этой матрицы с какой-нибудь другой, разумно конвертировать её в построчный непрерывный формат. Но копирование в лоб - достаточно дорогая операция и может стать хотспотом при большом объёме данных, поэтому нужны оптимизации.

## Оптимизации

### 1. Распараллеливание

Распараллеливание проводилось с помощью библиотеки Intel TBB. Матрица рабивается на блоки по строкам и каждый блок копируется в отдельном потоке. Число потоков - 4, как и количество физических ядер.

Размер блока был выбран исходя из объёма кеша L3 (8 MiB). Для матрицы типа `float` c 64 стобцами, 32 строки как раз занимают 8 MiB.

### 2. Инструкции AVX-512

Для ускорения операции копирования были использованы инструкции AVX-512:
* `_mm512_i64gather_ps` - собирает 8 чисел типа `float` и помещает их в один регистр
* `_mm256_storeu_ps` - копирует 8 чисел из регистра в непрерывный участок памяти

## Результаты

Замеры проводились на матрице типа `float` размером 4,194,304 x 64. Каждый эксперимент запускался по 10 раз

| Эксперимент | Среднее время, сек |
| ----------- | ------------- |
| Без оптимизаций | 0.189686 |
| Только распараллеливание | 0.0754772 |
| Распараллеливание + AVX-512 | 0.0365344 |

| Эксперимент | Ускорение |
| ----------- | ------------- |
| Только распараллеливание | 2.51315 |
| Распараллеливание + AVX-512 | 5.19198 |
