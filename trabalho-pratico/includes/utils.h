#ifndef UTILS_H
#define UTILS_H

#include <glib.h>
#include <stdio.h>

#define MASTER_DATE "09/10/2022"
#define MASTER_DATE_DATE \
  (struct date) { 9, 10, 2022 }

struct date {
  int day;
  int month;
  int year;
};

enum gender { M, F };

enum pay_method { CASH, CREDIT_CARD, DEBIT_CARD };

enum account_status { ACTIVE, INACTIVE };

enum car_class { BASIC, GREEN, PREMIUM };

char *create_filename(char *folder, const char *string);

const char *gender_to_string(int x);

const char *account_status_to_string(int x);

const char *car_class_to_string(int x);

const char *pay_method_to_string(int x);

int calculate_age(struct date birth_date);

int is_date_newer(struct date date1, struct date date2);

int is_date_equal(struct date date1, struct date date2);

int is_id_smaller(char *id1, char *id2);

int is_number(char *string);

int compare_dates(struct date date1, struct date date2);

char *date_to_string(struct date date);

char *strip(char *string);

gint compare_strings(gconstpointer a, gconstpointer b, gpointer data);

struct date increment_date(struct date date);

int maximum_day(struct date date);

struct date date_string_to_struct(char *date_string);

struct date date_int_to_struct(int date);

int date_struct_to_int(struct date date);

#endif