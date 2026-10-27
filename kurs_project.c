#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <unistd.h>

const char KDI[63] = {109, 110, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                      21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
                      42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60};

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

int compare_date(char *date1, char *date2)
{
    int d1, m1, y1;
    int d2, m2, y2;
    sscanf(date1, "%d-%d-%d", &d1, &m1, &y1);
    sscanf(date2, "%d-%d-%d", &d2, &m2, &y2);
    if (y1 != y2)
    {
        return y1 - y2;
    }
    if (m1 != m2)
    {
        return m1 - m2;
    }
    return d1 - d2;
}

typedef struct vertex
{
    Queue *data;
    struct vertex *left;
    struct vertex *right;
    int balance;
} vertex;

void obhod_left_right(vertex *v, Employee **index_array, int *index)
{
    if (v == NULL)
    {
        return;
    }
    obhod_left_right(v->left, index_array, index);

    spis *current = v->data->head;
    while (current != NULL)
    {
        index_array[*index] = &(current->data.employee);
        (*index)++;
        current = current->next;
    }

    obhod_left_right(v->right, index_array, index);
}

void turn_LL(vertex **p)
{
    vertex *q = (*p)->left;
    (*p)->balance = 0;
    q->balance = 0;
    (*p)->left = q->right;
    q->right = *p;
    *p = q;
}

void turn_LR(vertex **p)
{
    vertex *q = (*p)->left;
    vertex *r = q->right;
    if (r->balance < 0)
    {
        (*p)->balance = 1;
    }
    else
    {
        (*p)->balance = 0;
    }
    if (r->balance > 0)
    {
        q->balance = -1;
    }
    else
    {
        q->balance = 0;
    }
    r->balance = 0;
    q->right = r->left;
    (*p)->left = r->right;
    r->left = q;
    r->right = *p;
    *p = r;
}

void turn_RR(vertex **p)
{
    vertex *q = (*p)->right;
    (*p)->balance = 0;
    q->balance = 0;
    (*p)->right = q->left;
    q->left = *p;
    *p = q;
}

void turn_RL(vertex **p)
{
    vertex *q = (*p)->right;
    vertex *r = q->left;
    if (r->balance > 0)
    {
        (*p)->balance = -1;
    }
    else
    {
        (*p)->balance = 0;
    }
    if (r->balance < 0)
    {
        q->balance = 1;
    }
    else
    {
        q->balance = 0;
    }
    r->balance = 0;
    q->left = r->right;
    (*p)->right = r->left;
    r->left = *p;
    r->right = q;
    *p = r;
}

void add_to_avl(Employee adding_data, vertex **p, int *rost)
{
    if (*p == NULL)
    {
        *p = (vertex *)malloc(sizeof(vertex));
        (*p)->data = create_queue();
        add_to_queue((*p)->data, &adding_data);
        (*p)->left = NULL;
        (*p)->right = NULL;
        (*p)->balance = 0;
        *rost = 1;
    }
    else if (compare_date((*p)->data->head->data.employee.date_of_birth, adding_data.date_of_birth) > 0)
    {
        add_to_avl(adding_data, &((*p)->left), rost);
        if (*rost)
        {
            if ((*p)->balance > 0)
            {
                (*p)->balance = 0;
                *rost = 0;
            }
            else if ((*p)->balance == 0)
            {
                (*p)->balance = -1;
                *rost = 1;
            }
            else
            {
                if ((*p)->left->balance < 0)
                {
                    turn_LL(p);
                    *rost = 0;
                }
                else
                {
                    turn_LR(p);
                    *rost = 0;
                }
            }
        }
    }
    else if (compare_date((*p)->data->head->data.employee.date_of_birth, adding_data.date_of_birth) < 0)
    {
        add_to_avl(adding_data, &((*p)->right), rost);
        if (*rost)
        {
            if ((*p)->balance < 0)
            {
                (*p)->balance = 0;
                *rost = 0;
            }
            else if ((*p)->balance == 0)
            {
                (*p)->balance = 1;
                *rost = 1;
            }
            else
            {
                if ((*p)->right->balance > 0)
                {
                    turn_RR(p);
                    *rost = 0;
                }
                else
                {
                    turn_RL(p);
                    *rost = 0;
                }
            }
        }
    }
    else if (compare_date((*p)->data->head->data.employee.date_of_birth, adding_data.date_of_birth) == 0)
    {
        add_to_queue((*p)->data, &adding_data);
        *rost = 0;
    }
}

void delete_tree(vertex *v)
{
    if (v == NULL)
    {
        return;
    }
    delete_tree(v->left);
    delete_tree(v->right);
    clear_queue(v->data);
    free(v);
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

void print_index_page(Employee **arr, int page, int page_size, int total_count, char is_queue)
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
    if (is_queue)
    {
        printf("  СОДЕРЖИМОЕ ОЧЕРЕДИ\n");
    }
    else
    {
        printf("  СОДЕРЖИМОЕ ДЕРЕВА\n");
    }
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
    printf(" 1 - Вперед           | 2 - Назад                       | 3 - Быстрый поиск   | 4 - Отсортировать базу данных | 5 - Вывести страницу по номеру сотрудника\n");
    if (is_queue)
    {
        printf(" 6 - Построить дерево | 7 - Показать содержимое дерева  | 8 - Поиск по дереву | 0 - Выйти\n");
    }
    else
    {
        printf(" 6 - Построить дерево | 7 - Показать содержимое очереди | 8 - Поиск по дереву | 0 - Выйти\n");
    }
    printf(" Выберите действие: ");
}

int digital_sort(spis **S)
{
    int M = 0;
    for (int j = 64; j >= 0; j--)
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

void bin_search_v2(Employee **arr, int x, Queue *resQueue, int L, int R, int *total_found)
{
    int R_mem = R;
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
    {
        (*total_found)++;
        add_to_queue(resQueue, arr[L]);
        bin_search_v2(arr, x, resQueue, L + 1, R_mem, total_found);
    }
}

void search_and_build_queue(Employee **arr, int total_count, int target_day)
{
    int res_count = 0;
    Queue *resQueue = create_queue();
    bin_search_v2(arr, target_day, resQueue, 0, total_count - 1, &res_count);

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

Queue *search_in_tree(vertex **p, char *target_date)
{
    if (*p == NULL)
    {
        return NULL;
    }
    else if (compare_date((*p)->data->head->data.employee.date_of_birth, target_date) > 0)
    {
        return search_in_tree(&((*p)->left), target_date);
    }
    else if (compare_date((*p)->data->head->data.employee.date_of_birth, target_date) < 0)
    {
        return search_in_tree(&((*p)->right), target_date);
    }
    else if (compare_date((*p)->data->head->data.employee.date_of_birth, target_date) == 0)
    {
        return (*p)->data;
    }
}

void print_searched_in_tree(Queue *searched, char *target_date)
{
    if (searched == NULL)
    {
        searched = create_queue();
    }
    int res_count = 0;
    spis *curr_node = searched->head;
    while (curr_node != NULL)
    {
        res_count++;
        curr_node = curr_node->next;
    }
    Employee **search_array = (Employee **)malloc(res_count * sizeof(Employee *));
    curr_node = searched->head;
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
        printf("  РЕЗУЛЬТАТЫ ПОИСКА ПО ДЕРЕВУ | Страница %d из %d (Записи %d-%d из %d, Дата: %s)\n",
               search_page, total_search_pages, start_index + 1, end_index, res_count, target_date);
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
    if (searched->head == NULL)
    {
        clear_queue(searched);
    }
    free(search_array);
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
    Employee **tree_index_array = (Employee **)malloc(total_count * sizeof(Employee *));
    spis *curr = mainQueue->head;
    int index = 0;
    for (int i = 0; i < total_count; i++)
    {
        index_array[i] = &(curr->data.employee);
        curr = curr->next;
    }
    vertex *tree_root = NULL;
    int rost = 0;
    char is_queue = 1;
    int current_page = 1;
    int PAGE_SIZE = 20;
    int total_pages = (total_count + PAGE_SIZE - 1) / PAGE_SIZE;
    int choice;

    while (1)
    {
        if (is_queue)
        {
            print_index_page(index_array, current_page, PAGE_SIZE, total_count, is_queue);
        }
        else
        {
            print_index_page(tree_index_array, current_page, PAGE_SIZE, total_count, is_queue);
        }

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
        else if (choice == 5)
        {
            int emp_num;
            printf(" Введите номер сотрудника: ");
            if (scanf("%d", &emp_num) == 1)
            {
                if (emp_num <= 0)
                {
                    current_page = 1;
                }
                else if (emp_num >= total_count)
                {
                    current_page = total_count / PAGE_SIZE;
                }
                else
                {
                    current_page = (emp_num / PAGE_SIZE) + 1;
                }
            }
            else
            {
                while (getchar() != '\n')
                    ;
            }
        }
        else if (choice == 6)
        {
            delete_tree(tree_root);
            tree_root = NULL;
            curr = mainQueue->head;
            for (int i = 0; i < total_count; i++)
            {
                rost = 0;
                add_to_avl(curr->data.employee, &tree_root, &rost);
                curr = curr->next;
            }
            index = 0;
            obhod_left_right(tree_root, tree_index_array, &index);
        }
        else if (choice == 7)
        {
            if (tree_root != NULL)
            {
                is_queue = 1 - is_queue;
            }
            else
            {
                printf("Дерево еще не создано. Необходимо сначала построить. \nНажмите Enter для продолжения\n");
                sleep(1);
                int ch;
                while ((ch = getchar()) != '\n')
                    ;
                getchar();
            }
        }
        else if (choice == 8)
        {
            if (tree_root == NULL)
            {
                printf("Дерево еще не создано. Необходимо сначала построить.\nНажмите Enter для продолжения\n");
                sleep(1);
                int ch;
                while ((ch = getchar()) != '\n')
                    ;
                getchar();
            }
            else
            {
                char search_date[15];
                printf(" Введите дату рождения для поиска по дереву (формат 'ДД-ММ-ГГ'): ");
                if (scanf("%14s", search_date) == 1)
                {
                    Queue *found = search_in_tree(&tree_root, search_date);
                    print_searched_in_tree(found, search_date);
                }
                else
                {
                    while (getchar() != '\n')
                        ;
                }
            }
        }
        else if (choice == 0)
        {
            break;
        }
        else
        {
            printf("Введен некорректный номер действия.\nНажмите Enter для продолжения\n");
            sleep(1);
            int ch;
            while ((ch = getchar()) != '\n')
                ;
            getchar();
        }
    }

    system("clear");
    printf("Выход. Память кучи успешно очищена от %d записей.\n", total_count);

    free(index_array);
    clear_queue(mainQueue);
    free(tree_index_array);
    delete_tree(tree_root);

    return 0;
}
