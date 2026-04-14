/*
 * ============================================================
 *   SCHOOL BILLING SYSTEM
 *   Developed in C | KTU B.Tech Project
 * ============================================================
 *  Features:
 *    - Add / View / Search / Delete Students
 *    - Multiple Fee Categories: Tuition, Transport, Lab
 *    - Record Payments & Track Dues
 *    - Generate Invoice / Receipt
 *    - Persistent storage using text file (students.txt)
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ─── Constants ─────────────────────────────────────────── */
#define MAX_NAME      50
#define MAX_CLASS     20
#define DATA_FILE     "students.txt"

/* Fee amounts (in INR) — change as needed */
float FEE_TUITION = 15000.0f;   // now variable
#define FEE_TRANSPORT  3000.0f
#define FEE_LAB        2000.0f

/* ─── Data Structures ────────────────────────────────────── */
typedef struct {
    int   id;
    char  name[MAX_NAME];
    char  class_name[MAX_CLASS];

    /* Enrolled flags */
    int   has_transport;   /* 1 = yes, 0 = no */
    int   has_lab;         /* 1 = yes, 0 = no */

    /* Payment amounts */
    float total_due;
    float total_paid;
} Student;

/* ─── Utility: Clear Input Buffer ───────────────────────── */
void flush_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ─── Utility: Separator Lines (Windows-safe) ───────────── */
void print_double_line() {
    printf("  ==========================================\n");
}
void print_single_line() {
    printf("  ------------------------------------------\n");
}
void print_wide_double_line() {
    printf("  ===========================================================================\n");
}
void print_wide_single_line() {
    printf("  ---------------------------------------------------------------------------\n");
}
void print_menu_double_line() {
    printf("  =============================================\n");
}
void print_menu_single_line() {
    printf("  ---------------------------------------------\n");
}

/* ─── Calculate Total Due for a Student ─────────────────── */
float calculate_due(Student *s) {
    float total = FEE_TUITION;
    if (s->has_transport) total += FEE_TRANSPORT;
    if (s->has_lab)       total += FEE_LAB;
    return total;
}

/* ════════════════════════════════════════════════════════
   FILE I/O — Text Format
   Each record is stored as one line in students.txt:
   id|name|class_name|has_transport|has_lab|total_due|total_paid
   ════════════════════════════════════════════════════════ */

/* ─── Load All Students from TXT ────────────────────────── */
int load_students(Student arr[], int max) {
    FILE *fp = fopen(DATA_FILE, "r");
    if (!fp) return 0;

    int count = 0;
    char line[256];

    while (count < max && fgets(line, sizeof(line), fp)) {
        /* Remove trailing newline */
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        Student s;
        char name_buf[MAX_NAME];
        char class_buf[MAX_CLASS];

        /* Parse pipe-delimited fields */
        char *tok = strtok(line, "|");
        if (!tok) continue;
        s.id = atoi(tok);

        tok = strtok(NULL, "|");
        if (!tok) continue;
        strncpy(s.name, tok, MAX_NAME - 1);
        s.name[MAX_NAME - 1] = '\0';

        tok = strtok(NULL, "|");
        if (!tok) continue;
        strncpy(s.class_name, tok, MAX_CLASS - 1);
        s.class_name[MAX_CLASS - 1] = '\0';

        tok = strtok(NULL, "|");
        if (!tok) continue;
        s.has_transport = atoi(tok);

        tok = strtok(NULL, "|");
        if (!tok) continue;
        s.has_lab = atoi(tok);

        tok = strtok(NULL, "|");
        if (!tok) continue;
        s.total_due = (float)atof(tok);

        tok = strtok(NULL, "|");
        if (!tok) continue;
        s.total_paid = (float)atof(tok);

        arr[count++] = s;
    }

    fclose(fp);
    return count;
}

/* ─── Save All Students to TXT ──────────────────────────── */
void save_students(Student arr[], int count) {
    FILE *fp = fopen(DATA_FILE, "w");
    if (!fp) { printf("  [ERROR] Cannot open file for writing.\n"); return; }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d|%s|%s|%d|%d|%.2f|%.2f\n",
                arr[i].id,
                arr[i].name,
                arr[i].class_name,
                arr[i].has_transport,
                arr[i].has_lab,
                arr[i].total_due,
                arr[i].total_paid);
    }

    fclose(fp);
}

/* ─── Generate Next ID ───────────────────────────────────── */
int next_id(Student arr[], int count) {
    int max_id = 1000;
    for (int i = 0; i < count; i++)
        if (arr[i].id > max_id) max_id = arr[i].id;
    return max_id + 1;
}

/* ─── Find Student Index by ID ───────────────────────────── */
int find_by_id(Student arr[], int count, int id) {
    for (int i = 0; i < count; i++)
        if (arr[i].id == id) return i;
    return -1;
}
int is_duplicate(Student arr[], int count, char name[], char class_name[]) {
    for (int i = 0; i < count; i++) {
        if (strcmp(arr[i].name, name) == 0 &&
            strcmp(arr[i].class_name, class_name) == 0) {
            return 1;  // duplicate found
        }
    }
    return 0;
}

/* ─── Print Student Summary Row ─────────────────────────── */
void print_student_row(Student *s) {
    float balance = s->total_due - s->total_paid;
    printf("  %-6d %-25s %-12s  %9.2f  %9.2f  %9.2f\n",
           s->id, s->name, s->class_name,
           s->total_due, s->total_paid, balance);
}

/* ══════════════════════════════════════════════════════════
   FEATURE 1 — Add Student
   ══════════════════════════════════════════════════════════ */
void add_student(Student arr[], int *count) {
    if (*count >= 200) { printf("  [!] Student limit reached.\n"); return; }

    Student s;
    memset(&s, 0, sizeof(s));
    s.id = next_id(arr, *count);
printf("  Name        : "); fgets(s.name, MAX_NAME, stdin);
s.name[strcspn(s.name, "\n")] = '\0';

printf("  Class/Grade : "); fgets(s.class_name, MAX_CLASS, stdin);
s.class_name[strcspn(s.class_name, "\n")] = '\0';

/* 🔥 Duplicate check */
if (is_duplicate(arr, *count, s.name, s.class_name)) {
    printf("\n  [!] Student already exists (same name & class).\n");
    return;
}
printf("  Tuition Fee (INR): ");
scanf("%f", &FEE_TUITION);
flush_input();
    char ch;
    printf("  Transport fee? (y/n): "); scanf(" %c", &ch); flush_input();
    s.has_transport = (ch == 'y' || ch == 'Y') ? 1 : 0;

    printf("  Lab fee?       (y/n): "); scanf(" %c", &ch); flush_input();
s.has_lab = (ch == 'y' || ch == 'Y') ? 1 : 0;



/* Existing code */
s.total_due  = calculate_due(&s);
s.total_paid = 0.0f;

    arr[*count] = s;
    (*count)++;
    save_students(arr, *count);

    printf("\n  [OK] Student added. ID = %d  |  Total Due = %.2f\n", s.id, s.total_due);
}

/* ══════════════════════════════════════════════════════════
   FEATURE 2 — View All Students
   ══════════════════════════════════════════════════════════ */
void view_all(Student arr[], int count) {
    printf("\n");
    print_wide_double_line();
    printf("  %-6s %-25s %-12s  %9s  %9s  %9s\n",
           "ID", "Name", "Class", "Due(INR)", "Paid(INR)", "Bal(INR)");
    print_wide_single_line();
    if (count == 0) {
        printf("  No students found.\n");
    } else {
        for (int i = 0; i < count; i++)
            print_student_row(&arr[i]);
    }
    print_wide_double_line();
}

/* ══════════════════════════════════════════════════════════
   FEATURE 3 — Search Student
   ══════════════════════════════════════════════════════════ */
void search_student(Student arr[], int count) {
    int id;
    printf("\n  Enter Student ID to search: "); scanf("%d", &id); flush_input();
    int idx = find_by_id(arr, count, id);
    if (idx == -1) { printf("  [!] Student ID %d not found.\n", id); return; }

    Student *s = &arr[idx];
    printf("\n");
    print_single_line();
    printf("  Student Details\n");
    print_single_line();
    printf("  ID          : %d\n",    s->id);
    printf("  Name        : %s\n",    s->name);
    printf("  Class       : %s\n",    s->class_name);
    printf("  Tuition Fee : %.2f\n",  FEE_TUITION);
    printf("  Transport   : %s (%.2f)\n", s->has_transport ? "Yes" : "No",
                                           s->has_transport ? FEE_TRANSPORT : 0.0f);
    printf("  Lab Fee     : %s (%.2f)\n", s->has_lab ? "Yes" : "No",
                                           s->has_lab ? FEE_LAB : 0.0f);
    printf("  Total Due   : %.2f\n",  s->total_due);
    printf("  Total Paid  : %.2f\n",  s->total_paid);
    printf("  Balance     : %.2f\n",  s->total_due - s->total_paid);
    print_single_line();
}

/* ══════════════════════════════════════════════════════════
   FEATURE 4 — Record Payment
   ══════════════════════════════════════════════════════════ */
void record_payment(Student arr[], int count) {
    int id;
    printf("\n  Enter Student ID: "); scanf("%d", &id); flush_input();
    int idx = find_by_id(arr, count, id);
    if (idx == -1) { printf("  [!] Student ID %d not found.\n", id); return; }

    Student *s = &arr[idx];
    float balance = s->total_due - s->total_paid;
    printf("  Current Balance: %.2f\n", balance);

    if (balance <= 0) { printf("  [OK] No dues pending.\n"); return; }

    float amount;
    printf("  Payment Amount : "); scanf("%f", &amount); flush_input();

    if (amount <= 0) { printf("  [!] Invalid amount.\n"); return; }
    if (amount > balance) {
        printf("  [!] Amount exceeds balance. Capping to %.2f\n", balance);
        amount = balance;
    }

    s->total_paid += amount;
    save_students(arr, count);
    printf("  [OK] Payment of %.2f recorded. New balance: %.2f\n",
           amount, s->total_due - s->total_paid);
}

/* ══════════════════════════════════════════════════════════
   FEATURE 5 — Generate Invoice / Receipt
   ══════════════════════════════════════════════════════════ */
void generate_invoice(Student arr[], int count) {
    int id;
    printf("\n  Enter Student ID for Invoice: "); scanf("%d", &id); flush_input();
    int idx = find_by_id(arr, count, id);
    if (idx == -1) { printf("  [!] Student ID %d not found.\n", id); return; }

    Student *s = &arr[idx];

    /* Get current date/time */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date_str[30];
    strftime(date_str, sizeof(date_str), "%d-%m-%Y  %H:%M:%S", t);

    /* Build filename */
    char filename[40];
    snprintf(filename, sizeof(filename), "invoice_%d.txt", s->id);
    FILE *fp = fopen(filename, "w");
    if (!fp) { printf("  [ERROR] Could not create invoice file.\n"); return; }

    /* Print to both file and screen */
    FILE *outputs[2] = { stdout, fp };
    for (int o = 0; o < 2; o++) {
        FILE *out = outputs[o];
        fprintf(out, "\n");
        fprintf(out, "  ==========================================\n");
        fprintf(out, "       SCHOOL BILLING SYSTEM\n");
        fprintf(out, "            FEE INVOICE\n");
        fprintf(out, "  ==========================================\n");
        fprintf(out, "  Date        : %s\n", date_str);
        fprintf(out, "  Invoice No  : INV-%04d\n", s->id);
        fprintf(out, "  ------------------------------------------\n");
        fprintf(out, "  Student ID  : %d\n",   s->id);
        fprintf(out, "  Name        : %s\n",   s->name);
        fprintf(out, "  Class       : %s\n",   s->class_name);
        fprintf(out, "  ------------------------------------------\n");
        fprintf(out, "  %-22s  %10s\n", "Fee Category", "Amount (INR)");
        fprintf(out, "  ------------------------------------------\n");
        fprintf(out, "  %-22s  %10.2f\n", "Tuition Fee", FEE_TUITION);
        if (s->has_transport)
            fprintf(out, "  %-22s  %10.2f\n", "Transport Fee", FEE_TRANSPORT);
        if (s->has_lab)
            fprintf(out, "  %-22s  %10.2f\n", "Lab Fee", FEE_LAB);
        fprintf(out, "  ------------------------------------------\n");
        fprintf(out, "  %-22s  %10.2f\n", "TOTAL DUE",  s->total_due);
        fprintf(out, "  %-22s  %10.2f\n", "TOTAL PAID", s->total_paid);
        fprintf(out, "  %-22s  %10.2f\n", "BALANCE",    s->total_due - s->total_paid);
        fprintf(out, "  ==========================================\n");
        if (s->total_due - s->total_paid <= 0)
            fprintf(out, "  Status: *** FULLY PAID ***\n");
        else
            fprintf(out, "  Status: PAYMENT PENDING\n");
        fprintf(out, "  ==========================================\n\n");
    }

    fclose(fp);
    printf("  [OK] Invoice also saved to: %s\n", filename);
}

/* ══════════════════════════════════════════════════════════
   FEATURE 6 — Delete Student
   ══════════════════════════════════════════════════════════ */
void delete_student(Student arr[], int *count) {
    int id;
    printf("\n  Enter Student ID to delete: "); scanf("%d", &id); flush_input();
    int idx = find_by_id(arr, *count, id);
    if (idx == -1) { printf("  [!] Student ID %d not found.\n", id); return; }

    char confirm;
    printf("  Delete student '%s'? (y/n): ", arr[idx].name);
    scanf(" %c", &confirm); flush_input();
    if (confirm != 'y' && confirm != 'Y') { printf("  Cancelled.\n"); return; }

    /* Shift left */
    for (int i = idx; i < *count - 1; i++)
        arr[i] = arr[i + 1];
    (*count)--;
    save_students(arr, *count);
    printf("  [OK] Student deleted.\n");
}

/* ══════════════════════════════════════════════════════════
   MAIN MENU
   ══════════════════════════════════════════════════════════ */
void print_menu() {
    printf("\n");
    print_menu_double_line();
    printf("   SCHOOL BILLING SYSTEM -- MAIN MENU\n");
    print_menu_double_line();
    printf("   1. Add Student\n");
    printf("   2. View All Students\n");
    printf("   3. Search Student by ID\n");
    printf("   4. Record Payment\n");
    printf("   5. Generate Invoice / Receipt\n");
    printf("   6. Delete Student\n");
    printf("   0. Exit\n");
    print_menu_single_line();
    printf("   Choice: ");
}

/* ─── Entry Point ────────────────────────────────────────── */
int main() {
    Student students[200];
    int count = load_students(students, 200);
    printf("\n  [INFO] Loaded %d student(s) from file.\n", count);

    int choice;
    do {
        print_menu();
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        switch (choice) {
            case 1: add_student(students, &count);          break;
            case 2: view_all(students, count);              break;
            case 3: search_student(students, count);        break;
            case 4: record_payment(students, count);        break;
            case 5: generate_invoice(students, count);      break;
            case 6: delete_student(students, &count);       break;
            case 0: printf("\n  Goodbye!\n\n");             break;
            default: printf("  [!] Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}