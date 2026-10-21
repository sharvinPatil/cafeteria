#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 10
#define NAME_LEN 30
#define PHONE_LEN 15
#define ITEM_LEN 40
#define HISTORY_MAX 10
#define TABLE_SIZE 11
/* ================= STAGE 1 PRIORITY QUEUE ================= */
typedef struct {
    char item[ITEM_LEN];
    char customerName[NAME_LEN];
    int customerId;
    int priority;                 /* 1 = Teacher, 2 = Student, 3 = External */
} Order;
Order queue[MAX];
int orderCount = 0;
Order history[HISTORY_MAX];
int historyFront = 0, historyRear = 0, historyCount = 0;
void clearInput(void) {
    while (getchar() != '\n');
}
int readInt(const char *message) {
    int value;
    printf("%s", message);
    scanf("%d", &value);
    clearInput();
    return value;
}
const char *typeName(int type) {
    if (type == 1) return "Teacher/Staff";
    if (type == 2) return "Student";
    return "External";
}
int getType(void) {
    int type;
    do {
        printf("1. Teacher/Staff\n2. Student\n3. External\n");
        type = readInt("Enter type: ");
    } while (type < 1 || type > 3);
    return type;
}
void enqueue(Order order) {
    int i;
    if (orderCount == MAX) {
        printf("Order queue is full.\n");
        return;
    }
    queue[orderCount] = order;
    i = orderCount;
    /* Smaller priority number means higher priority. */
    while (i > 0 && queue[i].priority < queue[i - 1].priority) {
        Order temp = queue[i];
        queue[i] = queue[i - 1];
        queue[i - 1] = temp;
        i--;
    }
    orderCount++;
    history[historyRear] = order;
    historyRear = (historyRear + 1) % HISTORY_MAX;
    if (historyCount < HISTORY_MAX) historyCount++;
    else historyFront = (historyFront + 1) % HISTORY_MAX;
    printf("Order added successfully.\n");
}
void serveNext(void) {
    if (orderCount == 0) {
        printf("No orders available.\n");
        return;
    }
    printf("Serving %s's order: %s\n",
           queue[0].customerName, queue[0].item);
    for (int i = 0; i < orderCount - 1; i++)
        queue[i] = queue[i + 1];
    orderCount--;
}
/* ================= CUSTOMER LINKED LIST ================= */
typedef struct Customer {
    int id;
    int type;
    char name[NAME_LEN];
    char phone[PHONE_LEN];
    struct Customer *next;
} Customer;
Customer *customerHead = NULL;
void showHistory(Customer *customer) {
    int found = 0;
    printf("\n===== YOUR ORDER HISTORY =====\n");
    for (int i = 0; i < historyCount; i++) {
        int j = (historyFront + i) % HISTORY_MAX;
        if (history[j].customerId == customer->id)
            printf("%d. %s\n", ++found, history[j].item);
    }
    if (!found) printf("No order history found.\n");
}
void readText(char text[], int size) {
    fgets(text, size, stdin);
    text[strcspn(text, "\n")] = '\0';
}
/* ================= STAGE 3 HASH TABLE (CUSTOMER LOOKUP) ================= */
typedef struct HashNode {
    Customer *customer;
    struct HashNode *next;
} HashNode;
HashNode *table[TABLE_SIZE];
int hashFunction(int id) {
    return (id < 0 ? -id : id) % TABLE_SIZE;
}
Customer *findCustomer(int id) {
    for (HashNode *h = table[hashFunction(id)]; h != NULL; h = h->next)
        if (h->customer->id == id)
            return h->customer;
    return NULL;
}
void displayHashTable(void) {
    int empty = 1;
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (table[i] == NULL) continue;
        empty = 0;
        printf("Bucket %2d:", i);
        for (HashNode *h = table[i]; h != NULL; h = h->next)
            printf(" [%d, %s]", h->customer->id, h->customer->name);
        printf("\n");
    }
    if (empty) printf("Hash table is empty.\n");
}
void addCustomer(void) {
    int id = readInt("Enter customer ID: ");
    Customer *node;
    HashNode *h;
    int idx;
    if (findCustomer(id) != NULL) {
        printf("Customer ID already exists.\n");
        return;
    }
    node = malloc(sizeof(Customer));
    h = malloc(sizeof(HashNode));
    if (node == NULL || h == NULL) {
        free(node);
        free(h);
        printf("Memory allocation failed.\n");
        return;
    }
    node->id = id;
    printf("Enter name: ");
    readText(node->name, NAME_LEN);
    printf("Enter phone: ");
    readText(node->phone, PHONE_LEN);
    node->type = getType();
    node->next = customerHead;
    customerHead = node;
    idx = hashFunction(id);          /* keep hash table in sync with list */
    h->customer = node;
    h->next = table[idx];
    table[idx] = h;
    printf("Customer added successfully.\n");
}
void displayCustomers(void) {
    Customer *p = customerHead;
    if (p == NULL) {
        printf("No customer profiles found.\n");
        return;
    }
    printf("\nID\tName\t\tPhone\t\tType\n");
    while (p != NULL) {
        printf("%d\t%-15s %-15s %s\n",
               p->id, p->name, p->phone, typeName(p->type));
        p = p->next;
    }
}
void searchCustomer(void) {
    int id = readInt("Enter customer ID: ");
    Customer *p = findCustomer(id);          /* O(1) average hash lookup */
    if (p == NULL)
        printf("Customer not found.\n");
    else
        printf("ID: %d\nName: %s\nPhone: %s\nType: %s (bucket %d)\n",
               p->id, p->name, p->phone, typeName(p->type), hashFunction(id));
}
void updateCustomer(void) {
    Customer *p = findCustomer(readInt("Enter customer ID: "));
    if (p == NULL) {
        printf("Customer not found.\n");
        return;
    }
    printf("New name: ");
    readText(p->name, NAME_LEN);
    printf("New phone: ");
    readText(p->phone, PHONE_LEN);
    p->type = getType();
    printf("Customer updated successfully.\n");
}
void deleteCustomer(Customer *loggedIn) {
    int id = readInt("Enter customer ID: ");
    Customer *p = findCustomer(id), *previous = NULL, *cur = customerHead;
    int idx = hashFunction(id);
    HashNode *h, *hPrev = NULL;
    if (p == NULL) {
        printf("Customer not found.\n");
        return;
    }
    if (p == loggedIn) {
        printf("You cannot delete the logged-in profile.\n");
        return;
    }
    while (cur != p) {                       /* unlink from linked list */
        previous = cur;
        cur = cur->next;
    }
    if (previous == NULL)
        customerHead = p->next;
    else
        previous->next = p->next;
    for (h = table[idx]; h != NULL; hPrev = h, h = h->next)   /* unlink from hash chain */
        if (h->customer == p) {
            if (hPrev == NULL) table[idx] = h->next;
            else hPrev->next = h->next;
            free(h);
            break;
        }
    free(p);
    printf("Customer deleted successfully.\n");
}
void customerMenu(Customer *loggedIn) {
    int choice;
    do {
        printf("\n===== CUSTOMER PROFILES =====\n");
        printf("1. Add\n2. Display\n3. Search\n4. Update\n5. Delete\n6. Display Hash Table\n7. Back\n");
        choice = readInt("Choice: ");
        switch (choice) {
            case 1: addCustomer(); break;
            case 2: displayCustomers(); break;
            case 3: searchCustomer(); break;
            case 4: updateCustomer(); break;
            case 5: deleteCustomer(loggedIn); break;
            case 6: displayHashTable(); break;
            case 7: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 7);
}
void freeCustomers(void) {
    while (customerHead != NULL) {
        Customer *p = customerHead;
        customerHead = customerHead->next;
        free(p);
    }
    for (int i = 0; i < TABLE_SIZE; i++)
        while (table[i] != NULL) {
            HashNode *h = table[i];
            table[i] = h->next;
            free(h);
        }
}
Customer *login(void) {
    Customer *p = findCustomer(readInt("Enter customer ID to login: "));
    if (p == NULL)
        printf("Customer ID not found. Create a profile first.\n");
    else
        printf("Login successful. Welcome, %s!\n", p->name);
    return p;
}
/* ================= STACK NAVIGATION ================= */
typedef struct {
    char name[25];
} Menu;
Menu menuStack[MAX];
int menuTop = -1;
void pushMenu(const char *name) {
    if (menuTop < MAX - 1)
        strcpy(menuStack[++menuTop].name, name);
}
void backMenu(void) {
    if (menuTop > 0) {
        printf("Back: %s -> %s\n",
               menuStack[menuTop].name, menuStack[menuTop - 1].name);
        menuTop--;
    }
}
/* ================= CAFETERIA MENUS ================= */
char *veg[3][3] = {
    {"Paneer Tikka - Rs.120", "Veg Thali - Rs.150", "Masala Dosa - Rs.100"},
    {"Veg Manchurian - Rs.120", "Hakka Noodles - Rs.100", "Veg Fried Rice - Rs.110"},
    {"Tomato Soup - Rs.70", "Sweet Corn Soup - Rs.80", "Manchow Soup - Rs.90"}
};
char *nonVeg[3][3] = {
    {"Chicken Biryani - Rs.180", "Chicken Tikka - Rs.170", "Butter Chicken - Rs.200"},
    {"Chicken Noodles - Rs.160", "Chilli Chicken - Rs.190", "Chicken Fried Rice - Rs.170"},
    {"Chicken Soup - Rs.100", "Chicken Manchow - Rs.120", "Egg Drop Soup - Rs.100"}
};
void addOrder(const char *item, Customer *customer) {
    Order order;
    order.customerId = customer->id;
    order.priority = customer->type;
    strcpy(order.item, item);
    strcpy(order.customerName, customer->name);
    printf("Customer: %s | Priority: %s\n",
           customer->name, typeName(customer->type));
    enqueue(order);
}
void dishes(const char *title, char *items[], Customer *customer) {
    int choice;
    do {
        printf("\n===== %s =====\n", title);
        for (int i = 0; i < 3; i++)
            printf("%d. %s\n", i + 1, items[i]);
        printf("4. Back\n");
        choice = readInt("Choice: ");
        if (choice >= 1 && choice <= 3)
            addOrder(items[choice - 1], customer);
        else if (choice != 4)
            printf("Invalid choice.\n");
    } while (choice != 4);
}
int category(const char *title) {
    printf("\n===== %s =====\n1. Indian\n2. Chinese\n3. Soup\n4. Back\n", title);
    return readInt("Choice: ");
}
/* ================= STAGE 3 BST (DISH CATALOG) ================= */
typedef struct Dish {
    char name[NAME_LEN];
    int price;
    struct Dish *left, *right;
} Dish;
Dish *dishRoot = NULL;
Dish *createDish(const char *name, int price) {
    Dish *node = malloc(sizeof(Dish));
    if (node == NULL) return NULL;
    strncpy(node->name, name, NAME_LEN - 1);
    node->name[NAME_LEN - 1] = '\0';
    node->price = price;
    node->left = node->right = NULL;
    return node;
}
Dish *insertDish(Dish *node, const char *name, int price, int *added) {
    int cmp;
    if (node == NULL) {
        Dish *created = createDish(name, price);
        *added = (created != NULL);
        return created;
    }
    cmp = strcmp(name, node->name);
    if (cmp < 0)
        node->left = insertDish(node->left, name, price, added);
    else if (cmp > 0)
        node->right = insertDish(node->right, name, price, added);
    else
        *added = 0;
    return node;
}
Dish *findDish(Dish *node, const char *name) {
    while (node != NULL) {
        int cmp = strcmp(name, node->name);
        if (cmp == 0) return node;
        node = (cmp < 0) ? node->left : node->right;
    }
    return NULL;
}
Dish *minDish(Dish *node) {
    while (node->left != NULL)
        node = node->left;
    return node;
}
Dish *deleteDishNode(Dish *node, const char *name, int *removed) {
    int cmp;
    if (node == NULL) return NULL;
    cmp = strcmp(name, node->name);
    if (cmp < 0) {
        node->left = deleteDishNode(node->left, name, removed);
    } else if (cmp > 0) {
        node->right = deleteDishNode(node->right, name, removed);
    } else {
        *removed = 1;
        if (node->left == NULL) {
            Dish *child = node->right;
            free(node);
            return child;
        }
        if (node->right == NULL) {
            Dish *child = node->left;
            free(node);
            return child;
        }
        Dish *succ = minDish(node->right);          /* inorder successor */
        int dummy = 0;
        strcpy(node->name, succ->name);
        node->price = succ->price;
        node->right = deleteDishNode(node->right, succ->name, &dummy);
    }
    return node;
}
void inorderDishes(Dish *node) {
    if (node == NULL) return;
    inorderDishes(node->left);
    printf("  %-22s Rs.%d\n", node->name, node->price);
    inorderDishes(node->right);
}
void preorderDishes(Dish *node, int depth) {
    if (node == NULL) return;
    printf("%*s%s\n", depth * 4, "", node->name);
    preorderDishes(node->left, depth + 1);
    preorderDishes(node->right, depth + 1);
}
int dishHeight(Dish *node) {
    int l, r;
    if (node == NULL) return 0;
    l = dishHeight(node->left);
    r = dishHeight(node->right);
    return 1 + (l > r ? l : r);
}
void freeDishes(Dish *node) {
    if (node == NULL) return;
    freeDishes(node->left);
    freeDishes(node->right);
    free(node);
}
/* A dish cannot be deleted while it is waiting in the order queue. */
int dishInUse(const char *name) {
    size_t len = strlen(name);
    for (int i = 0; i < orderCount; i++)
        if (strncmp(queue[i].item, name, len) == 0 && queue[i].item[len] == ' ')
            return 1;
    return 0;
}
/* Items are stored as "Name - Rs.120"; split them into name and price. */
void loadDish(const char *text) {
    char name[NAME_LEN];
    const char *sep = strstr(text, " - Rs.");
    int added = 0;
    if (sep == NULL) return;
    strncpy(name, text, sep - text);
    name[sep - text] = '\0';
    dishRoot = insertDish(dishRoot, name, atoi(sep + 6), &added);
}
void loadDishes(void) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            loadDish(veg[i][j]);
            loadDish(nonVeg[i][j]);
        }
}
void addDish(void) {
    char name[NAME_LEN];
    int price, added = 0;
    printf("Enter dish name: ");
    readText(name, NAME_LEN);
    if (name[0] == '\0') {
        printf("Name cannot be empty.\n");
        return;
    }
    price = readInt("Enter price: ");
    dishRoot = insertDish(dishRoot, name, price, &added);
    printf(added ? "Dish added.\n" : "Dish already exists.\n");
}
void searchDish(void) {
    char name[NAME_LEN];
    Dish *d;
    printf("Enter dish name to search: ");
    readText(name, NAME_LEN);
    d = findDish(dishRoot, name);
    if (d != NULL) printf("Dish found: %s - Rs.%d\n", d->name, d->price);
    else printf("Dish not found.\n");
}
void deleteDish(void) {
    char name[NAME_LEN];
    int removed = 0;
    printf("Enter dish name to delete: ");
    readText(name, NAME_LEN);
    if (dishInUse(name)) {
        printf("Cannot delete: dish is in the order queue.\n");
        return;
    }
    dishRoot = deleteDishNode(dishRoot, name, &removed);
    printf(removed ? "Dish deleted.\n" : "Dish not found.\n");
}
void displayDishes(void) {
    if (dishRoot == NULL) {
        printf("No dishes available.\n");
        return;
    }
    printf("\nDishes (alphabetical):\n");
    inorderDishes(dishRoot);
    printf("\nHierarchy (root first, height %d):\n", dishHeight(dishRoot));
    preorderDishes(dishRoot, 0);
}
void dishMenu(void) {
    int choice;
    do {
        printf("\n===== DISH CATALOG (BST) =====\n");
        printf("1. Add Dish\n2. Search Dish\n3. Delete Dish\n4. Display Dishes\n5. Back\n");
        choice = readInt("Choice: ");
        switch (choice) {
            case 1: addDish(); break;
            case 2: searchDish(); break;
            case 3: deleteDish(); break;
            case 4: displayDishes(); break;
            case 5: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 5);
}
/* ================= MAIN ================= */
int main(void) {
    int choice, categoryChoice, current = 1, running = 1;
    Customer *loggedIn = NULL;
    pushMenu("Main Menu");
    loadDishes();
    while (running) {
        if (loggedIn == NULL) {
            printf("\n===== CAFETERIA LOGIN =====\n");
            printf("1. Login using Customer ID\n2. Create Customer Profile\n3. Exit\n");
            choice = readInt("Choice: ");
            if (choice == 1) loggedIn = login();
            else if (choice == 2) addCustomer();
            else if (choice == 3) running = 0;
            else printf("Invalid choice.\n");
            continue;
        }
        if (current == 1) {
            printf("\n===== CAFETERIA SYSTEM =====\nLogged in: %s\n", loggedIn->name);
            printf("1. Veg Menu\n2. Non-Veg Menu\n3. Customer Profiles\n");
            printf("4. Serve Next Order\n5. Order History\n6. Dish Catalog\n");
            printf("7. Logout\n8. Exit\n");
            choice = readInt("Choice: ");
            if (choice == 1) {
                pushMenu("Veg Menu");
                current = 2;
            } else if (choice == 2) {
                pushMenu("Non-Veg Menu");
                current = 3;
            } else if (choice == 3) {
                pushMenu("Customer Profiles");
                current = 4;
            } else if (choice == 4) {
                serveNext();
            } else if (choice == 5) {
                showHistory(loggedIn);
            } else if (choice == 6) {
                pushMenu("Dish Catalog");
                current = 5;
            } else if (choice == 7) {
                loggedIn = NULL;
                current = 1;
                menuTop = 0;
                printf("Logged out.\n");
            } else if (choice == 8) {
                running = 0;
            } else {
                printf("Invalid choice.\n");
            }
        } else if (current == 2) {
            categoryChoice = category("VEG MENU");
            if (categoryChoice == 1)
                dishes("VEG - INDIAN", veg[0], loggedIn);
            else if (categoryChoice == 2)
                dishes("VEG - CHINESE", veg[1], loggedIn);
            else if (categoryChoice == 3)
                dishes("VEG - SOUP", veg[2], loggedIn);
            else if (categoryChoice == 4) {
                backMenu();
                current = 1;
            } else printf("Invalid choice.\n");
        } else if (current == 3) {
            categoryChoice = category("NON-VEG MENU");
            if (categoryChoice == 1)
                dishes("NON-VEG - INDIAN", nonVeg[0], loggedIn);
            else if (categoryChoice == 2)
                dishes("NON-VEG - CHINESE", nonVeg[1], loggedIn);
            else if (categoryChoice == 3)
                dishes("NON-VEG - SOUP", nonVeg[2], loggedIn);
            else if (categoryChoice == 4) {
                backMenu();
                current = 1;
            } else printf("Invalid choice.\n");
        } else if (current == 4) {
            customerMenu(loggedIn);
            backMenu();
            current = 1;
        } else {
            dishMenu();
            backMenu();
            current = 1;
        }
    }
    freeCustomers();
    freeDishes(dishRoot);
    return 0;
}
