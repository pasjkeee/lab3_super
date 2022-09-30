# lab3_super

Рыбкин Павел м21-502 


Приложение представляет себя клиент сервер на базе tcp соект соединения

## Сервер
В первую очередь запускается сервер и вводится количество мегабайт сгенерированных данных
Генрируется матрица с цифрами от 0 до 100 
Колчиество стро и столбцов в зависимости от мегайбайт высичтывается по формуле 
int res = (dataInMb * 1024) / sqrt(3*dataInMb);
По сути в среднем имеем 3 бита на одно число (пробел + само число в 90: случаев состоящее из 2х цифр)

Сгенерированные данные грузятся файл

Для старта работы сервер должен принять от клиента команду "start"

После чего данные берутся с файла по 80 симоволов и отправляются к клиенту


Сервер принимает от клиента команду то что он начал отпарвлять даннные "start" получившиеся цифры вектора

При команде exit будет сформировано время выполнения а так же принят размео буффера

Все записывается в res.txt

Данные егнерируются в matrix.txt

## Клиент
Клиент для запуска должен отправить команду "start"

Парсинг происходит по алгоритмму:

Если пробел - значит число закончилось => преобразоввываем val в число

Если '\n' то это переност строки => чистаем среднюю

Если не пробел не \n и не \t => формируем строку числа

Если e => уонец приема

## Приницип работы

MPI использует объекты называемые коммуникаторы

+ Defines which processes can talk

+ Communicators have a size


MPI_COMM_WORLD

+ Predefined as ALL of the MPI Processes

+ Size = Nprocs

Rank

+ Integer process identifier

+ 0≤Rank<Size

MPI_Comm_size //количество процессов

MPI_Comm_rank //текущий process id


mpicc -o main ./main.c

mpirun -np 4 ./main.c



### График времени выполнения

| Метод.         | Время (мс)на 10мб  | Время (мс)на 50мб  | Время (мс)на 100мб | Время (мс)на 500мб | Время (мс)на 1000мб|
| -------------- |:------------------:|:------------------:|:------------------:|:------------------:|:------------------:|
| Линейный       |         7          |         33         |        74          |       539          |       1287         |
| OMPI 4 thread  |                    |                    |                    |                    |                    |
| OMPI 6 thread  |                    |                    |                    |                    |                    |



