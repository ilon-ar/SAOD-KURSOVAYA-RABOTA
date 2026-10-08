#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>

const char KDI[6] = {109, 110, 0, 1, 2, 3};

#pragma pack(push, 1)
typedef struct
{
    char FIO[30];
    short department;
    char position[22];
    char date_of_birth[10];
} FileEmployee;
#pragma pack(pop)

typedef struct
{
    char FIO[61];
    short department;
    char position[45];
    char date_of_birth[13];
} Employee;

typedef struct spis
{
    struct spis *next;
    union
    {
        Employee employee;
        unsigned char raw_data[121];
    } data;
} spis;

typedef struct
{
    spis *head;
    spis *tail;
} Queue;

Queue *create_queue()
{
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->head = NULL;
    q->tail = (spis *)(&(q->head));
    return q;
}

void add_to_queue(Queue *q, Employee *emp)
{
    spis *tmp = (spis *)malloc(sizeof(spis));
    tmp->data.employee = *emp;
    tmp->next = NULL;

    *((spis **)(q->tail)) = tmp;
    q->tail = (spis *)&(tmp->next);
}

void clear_queue(Queue *q)
{
    if (q == NULL)
        return;
    spis *current = q->head;
    while (current != NULL)
    {
        spis *nextNode = current->next;
        free(current);
        current = nextNode;
    }
    free(q);
}

void convert_cp866_to_utf8(char *source, size_t sourceLen, char *dest, size_t destMaxLen)
{
    iconv_t cd = iconv_open("UTF-8", "CP866");
    if (cd == (iconv_t)-1)
    {
        cd = iconv_open("UTF-8", "CP1251");
    }

    if (cd == (iconv_t)-1)
    {
        int i = 0;
        for (; i < sourceLen && i < destMaxLen - 1; ++i)
            dest[i] = source[i];
        dest[i] = '\0';
        return;
    }

    size_t inBytesLeft = sourceLen;
    size_t outBytesLeft = destMaxLen - 1;
    char *inPtr = (char *)source;
    char *outPtr = dest;

    iconv(cd, &inPtr, &inBytesLeft, &outPtr, &outBytesLeft);
    iconv_close(cd);
    *outPtr = '\0';
}

int get_utf8_visible_width(char *str)
{
    int width = 0;
    while (*str)
    {
        unsigned char c = (unsigned char)*str;
        if ((c & 0xC0) != 0x80)
            width++;
        str++;
    }
    return width;
}

void print_column(char *str, int target_width)
{
    printf("%s", str);
    int visible_width = get_utf8_visible_width(str);
    int spaces_to_print = target_width - visible_width;
    for (int i = 0; i < spaces_to_print; i++)
        putchar(' ');
}

void print_index_page(Employee **arr, int page, int page_size, int total_count)
{
    if (arr == NULL || total_count == 0)
    {
        printf("Информационное сообщение: База данных пуста.\n");
        return;
    }

    int total_pages = (total_count + page_size - 1) / page_size;
    int start_index = (page - 1) * page_size;
    int end_index = start_index + page_size;
    if (end_index > total_count)
        end_index = total_count;

    system("clear");

    printf("=========================================================================================================\n");
    printf("  БАЗА ДАННЫХ КУРСА НА 4000 ПЕРСОН | Страница %d из %d (Записи %d-%d из %d)\n",
           page, total_pages, start_index + 1, end_index, total_count);
    printf("=========================================================================================================\n");

    printf("---------------------------------------------------------------------------------------------------------\n");
    printf("| ");
    print_column("№", 4);
    printf(" | ");
    print_column("ФИО сотрудника", 35);
    printf(" | ");
    print_column("Отдел", 10);
    printf(" | ");
    print_column("Должность", 30);
    printf(" | ");
    print_column("Дата рожд.", 12);
    printf(" |\n");
    printf("---------------------------------------------------------------------------------------------------------\n");

    for (int i = start_index; i < end_index; i++)
    {
        printf("| %-4d | ", i + 1);
        print_column(arr[i]->FIO, 35);
        printf(" | %-10d | ", arr[i]->department);
        print_column(arr[i]->position, 30);
        printf(" | %-12s |\n", arr[i]->date_of_birth);
    }

    printf("---------------------------------------------------------------------------------------------------------\n");
    printf(" 1 - Вперед | 2 - Назад | 3 - Быстрый поиск | 0 - Выйти\n");
    printf(" Выберите действие: ");
}

int digital_sort(spis **S)
{
    int M = 0;
    for (int j = 5; j >= 0; j--)
    {
        Queue Q[256];
        for (int i = 0; i < 256; i++)
        {
            Q[i].tail = (spis *)&(Q[i].head);
            Q[i].head = NULL;
        }
        spis *p = *S;
        char k = KDI[j];
        while (p != NULL)
        {
            unsigned char d = p->data.raw_data[k];
            Q[d].tail->next = p;
            Q[d].tail = p;
            p = p->next;
            M++;
        }
        spis tmp;
        p = &tmp;
        for (int i = 0; i < 256; i++)
        {
            if (Q[i].tail != (spis *)&(Q[i].head))
            {
                p->next = Q[i].head;
                p = Q[i].tail;
            }
        }
        p->next = NULL;
        *S = tmp.next;
    }
    return M;
}

int bin_search_v2(Employee **arr, int n, int x)
{
    int L = 0, R = n - 1;
    int m;
    while (L < R)
    {
        m = (L + R) / 2;
        if (atoi(arr[m]->date_of_birth) < x)
            L = m + 1;
        else
            R = m;
    }
    if (atoi(arr[L]->date_of_birth) == x)
        return L;
    else
        return -1;
}

void search_and_build_queue(Employee **arr, int total_count, int target_day)
{
    int first_idx = bin_search_v2(arr, total_count, target_day);

    if (first_idx == -1)
    {
        printf("\nСотрудники с днем рождения %02d не найдены.\n", target_day);
        printf("Нажмите Enter для продолжения...");
        while (getchar() != '\n')
            ;
        getchar();
        return;
    }
    Queue *resQueue = create_queue();
    int res_count = 0;
    int i = first_idx;
    while (i < total_count && atoi(arr[i]->date_of_birth) == target_day)
    {
        add_to_queue(resQueue, arr[i]);
        i++;
        res_count++;
    }
    Employee **search_array = (Employee **)malloc(res_count * sizeof(Employee *));
    spis *curr_node = resQueue->head;
    for (int j = 0; j < res_count; j++)
    {
        search_array[j] = &(curr_node->data.employee);
        curr_node = curr_node->next;
    }

    int search_page = 1;
    int PAGE_SIZE = 20;
    int total_search_pages = (res_count + PAGE_SIZE - 1) / PAGE_SIZE;
    int search_choice;
    while (1)
    {
        int start_index = (search_page - 1) * PAGE_SIZE;
        int end_index = start_index + PAGE_SIZE;
        if (end_index > res_count)
            end_index = res_count;

        system("clear");
        printf("=========================================================================================================\n");
        printf("  РЕЗУЛЬТАТЫ БЫСТРОГО ПОИСКА | Страница %d из %d (Записи %d-%d из %d, День: %02d)\n",
               search_page, total_search_pages, start_index + 1, end_index, res_count, target_day);
        printf("=========================================================================================================\n");

        printf("---------------------------------------------------------------------------------------------------------\n");
        printf("| ");
        print_column("№", 4);
        printf(" | ");
        print_column("ФИО сотрудника", 35);
        printf(" | ");
        print_column("Отдел", 10);
        printf(" | ");
        print_column("Должность", 30);
        printf(" | ");
        print_column("Дата рожд.", 12);
        printf(" |\n");
        printf("---------------------------------------------------------------------------------------------------------\n");

        for (int k = start_index; k < end_index; k++)
        {
            printf("| %-4d | ", k + 1);
            print_column(search_array[k]->FIO, 35);
            printf(" | %-10d | ", search_array[k]->department);
            print_column(search_array[k]->position, 30);
            printf(" | %-12s |\n", search_array[k]->date_of_birth);
        }
        printf("---------------------------------------------------------------------------------------------------------\n");
        printf(" 1 - Вперед | 2 - Назад | 0 - Вернуться в главное меню\n");
        printf(" Выберите действие: ");

        if (scanf("%d", &search_choice) != 1)
        {
            while (getchar() != '\n')
                ;
            break;
        }

        if (search_choice == 1)
        {
            if (search_page < total_search_pages)
                search_page++;
        }
        else if (search_choice == 2)
        {
            if (search_page > 1)
                search_page--;
        }
        else if (search_choice == 0)
        {
            break;
        }
    }

    free(search_array);

    spis *p = resQueue->head;
    while (p != NULL)
    {
        spis *next = p->next;
        free(p);
        p = next;
    }
    free(resQueue);
}

int main()
{
    FILE *file = fopen("testBase2.dat", "rb");
    if (file == NULL)
    {
        printf("Ошибка: не удалось открыть базу данных!\n");
        return 1;
    }

    Queue *mainQueue = create_queue();
    FileEmployee rawEmp;
    int total_count = 0;

    while (fread(&rawEmp, sizeof(FileEmployee), 1, file) == 1)
    {
        Employee utf8Emp;

        convert_cp866_to_utf8(rawEmp.FIO, 30, utf8Emp.FIO, sizeof(utf8Emp.FIO));
        convert_cp866_to_utf8(rawEmp.position, 22, utf8Emp.position, sizeof(utf8Emp.position));

        int writeIdx = 0;
        for (int readIdx = 0; readIdx < 10; ++readIdx)
        {
            char ch = rawEmp.date_of_birth[readIdx];
            if ((ch >= '0' && ch <= '9') || ch == '-')
            {
                utf8Emp.date_of_birth[writeIdx++] = ch;
            }
        }
        utf8Emp.date_of_birth[writeIdx] = '\0';
        utf8Emp.department = rawEmp.department;

        add_to_queue(mainQueue, &utf8Emp);
        total_count++;
    }
    fclose(file);

    Employee **index_array = (Employee **)malloc(total_count * sizeof(Employee *));
    spis *curr = mainQueue->head;
    for (int i = 0; i < total_count; i++)
    {
        index_array[i] = &(curr->data.employee);
        curr = curr->next;
    }

    int current_page = 1;
    int PAGE_SIZE = 20;
    int total_pages = (total_count + PAGE_SIZE - 1) / PAGE_SIZE;
    int choice;

    while (1)
    {
        print_index_page(index_array, current_page, PAGE_SIZE, total_count);

        if (scanf("%d", &choice) != 1)
        {
            while (getchar() != '\n')
                ;
            break;
        }

        if (choice == 1)
        {
            if (current_page < total_pages)
                current_page++;
        }
        else if (choice == 2)
        {
            if (current_page > 1)
                current_page--;
        }
        else if (choice == 3)
        {
            int search_day;
            printf(" Введите день рождения для поиска (1-31): ");
            if (scanf("%d", &search_day) == 1)
            {
                search_and_build_queue(index_array, total_count, search_day);
            }
            else
            {
                while (getchar() != '\n')
                    ;
            }
        }
        else if (choice == 4)
        {
            digital_sort(&(mainQueue->head));
            curr = mainQueue->head;
            for (int i = 0; i < total_count; i++)
            {
                index_array[i] = &(curr->data.employee);
                curr = curr->next;
            }
        }
        else
        {
            break;
        }
    }

    system("clear");
    printf("Выход. Память кучи успешно очищена от %d записей.\n", total_count);

    free(index_array);
    clear_queue(mainQueue);

    return 0;
}
