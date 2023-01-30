#include "querier.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "catalog.h"
#include "drivers.h"
#include "rides.h"
#include "stats.h"
#include "users.h"
#include "utils.h"

void querier(CATALOG catalog, STATS stats, char *line, int counter) {
  edit_strip(line);
  char **query_parameter = malloc(sizeof(char *) * MAX_INPUT_TOKENS);
  char *token = strtok(line, " ");

  int query_number = atoi(token);
  token = strtok(NULL, " ");

  int i = 0;
  while (token) {
    query_parameter[i] = token;

    token = strtok(NULL, " ");
    i++;
  }

  function_pointer table[] = {query1, query2, query3, query4, query5,
                              query6, query7, query8, query9};

  table[query_number - 1](catalog, stats, query_parameter, counter);
}

void query1(CATALOG catalog, STATS stats, char **parameter, int counter) {
  clock_t begin = clock();

  char *id = parameter[0];
  id[strlen(id) - 1] = '\0';

  int flag = is_number(id);

  switch (flag) {
    case 0:
      get_user_profile(catalog, id, counter);
      break;
    case 1:
      get_driver_profile(catalog, id, counter);
  }

  free(parameter);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("%d: Query 1 elapsed time: %f seconds\n", counter, time_spent);

  // Since stats is not used in this query, we can ignore the warning
  (void)stats;
}

void query2(CATALOG catalog, STATS stats, char **parameter, int counter) {
  clock_t begin = clock();

  int n;
  sscanf(parameter[0], "%d", &n);

  char *output_filename = malloc(sizeof(char) * 256);
  sprintf(output_filename, "Resultados/command%d_output.txt", counter);

  FILE *output_file = fopen(output_filename, "a");

  if (output_file == NULL) {
    printf("Error creating command%d_output.txt file\n", counter);
    return;
  }

  static int is_drivers_sorted = 0;
  GArray *top_drivers = get_top_drivers_by_average_score(stats);

  if (!is_drivers_sorted) {
    qsort(top_drivers->data, top_drivers->len, sizeof(DRIVER),
          (GCompareFunc)compare_drivers_by_average_score);
    is_drivers_sorted = 1;
  }

  int i = 0;
  while (n > 0 && i < (int)top_drivers->len) {
    DRIVER driver = g_array_index(top_drivers, DRIVER, i);
    int driver_id = get_driver_id(driver);
    char *name = get_driver_name(driver);

    double total_rating = get_driver_total_rating(driver);
    int number_of_rides = get_driver_number_of_rides(driver);
    double average_score = (double)(total_rating / number_of_rides);

    enum account_status account_status = get_driver_account_status(driver);

    if (account_status == ACTIVE) {
      fprintf(output_file, "%012d;%s;%.3f\n", driver_id, name, average_score);
      n--;
    }

    ++i;
    free(name);
  }

  free(parameter);
  free(output_filename);
  fclose(output_file);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("%d: Query 2 elapsed time: %f seconds\n", counter, time_spent);

  (void)catalog;
}

void query3(CATALOG catalog, STATS stats, char **parameter, int counter) {
  clock_t begin = clock();

  int n;
  sscanf(parameter[0], "%d", &n);

  char *output_filename = malloc(sizeof(char) * 256);
  sprintf(output_filename, "Resultados/command%d_output.txt", counter);

  FILE *output_file = fopen(output_filename, "a");

  if (output_file == NULL) {
    printf("Error creating command%d_output.txt file\n", counter);
    return;
  }

  static int is_users_sorted = 0;
  GArray *top_users = get_top_users_by_total_distance(stats);
  GPtrArray *users_reverse_lookup = get_catalog_users_reverse_lookup(catalog);

  if (!is_users_sorted) {
    g_array_sort_with_data(top_users,
                           (GCompareDataFunc)compare_users_by_total_distance,
                           users_reverse_lookup);
    is_users_sorted = 1;
  }

  int i = 0;
  while (n > 0 && i < (int)top_users->len) {
    USER user = g_array_index(top_users, USER, i);
    int username = get_user_username(user);
    char *name = get_user_name(user);

    int total_distance = get_user_total_distance(user);
    enum account_status account_status = get_user_account_status(user);

    if (account_status == ACTIVE) {
      char *username_string = g_ptr_array_index(users_reverse_lookup, username);
      fprintf(output_file, "%s;%s;%d\n", username_string, name, total_distance);
      n--;
    }

    ++i;
    free(name);
  }

  free(parameter);
  free(output_filename);
  fclose(output_file);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("%d: Query 3 elapsed time: %f seconds\n", counter, time_spent);

  (void)catalog;
}

void query4(CATALOG catalog, STATS stats, char **parameter, int counter) {
  clock_t begin = clock();

  char *output_filename = malloc(sizeof(char) * 256);
  sprintf(output_filename, "Resultados/command%d_output.txt", counter);

  FILE *output_file = fopen(output_filename, "a");

  if (output_file == NULL) {
    printf("Error creating command%d_output.txt file\n", counter);
    return;
  }

  char *city_string = parameter[0];
  city_string[strlen(city_string) - 1] = '\0';
  GHashTable *city_code = get_catalog_city_code(catalog);
  char *city_number = g_hash_table_lookup(city_code, city_string);

  if (city_number) {
    int city = *city_number;

    CITY_STATS city_stats = get_city_stats(stats, city);

    if (city_stats != NULL) {
      double total_spent = get_city_stats_total_spent(city_stats);
      int total_rides = get_city_stats_total_rides(city_stats);

      double result = (double)(total_spent / total_rides);

      fprintf(output_file, "%.3f\n", result);
    }
  }

  free(parameter);
  free(output_filename);
  fclose(output_file);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("%d: Query 4 elapsed time: %f seconds\n", counter, time_spent);

  // Since catalog is not used in this query, we can safely ignore the warning
  (void)catalog;
}

gboolean tree_to_array(gpointer key, gpointer value, gpointer user_data) {
  GPtrArray *array = (GPtrArray *)user_data;
  g_ptr_array_add(array, value);
  return FALSE;

  (void)key;
}

void query5(CATALOG catalog, STATS stats, char **parameter, int counter) {
  query5_6(catalog, stats, parameter, counter, 0);
}

void query6(CATALOG catalog, STATS stats, char **parameter, int counter) {
  query5_6(catalog, stats, parameter, counter, 1);
}

void query5_6(CATALOG catalog, STATS stats, char **parameters, int counter,
              int query6_determiner) {  // If query_determiner is 0, query5 is
                                        // made, if it's 1, query 6 is made
  clock_t begin = clock();
  double average = 0.f;
  GHashTable *rides_by_date = get_rides_by_date(stats);

  char *city = NULL;

  if (query6_determiner) {
    city = parameters[0];  // If we are calculating query6, the city has a value
  }

  int lower_limit = date_string_to_int(parameters[0 + query6_determiner]);
  int upper_limit = date_string_to_int(parameters[1 + query6_determiner]);

  if (query6_determiner) {
    GHashTable *city_codes = get_catalog_city_code(catalog);
    char *city_code;
    city_code = g_hash_table_lookup(city_codes, city);
    if (city_code)
      average = calculate_average_distance(rides_by_date, lower_limit,
                                           upper_limit, *city_code);
  } else {
    average = calculate_average_price(rides_by_date, lower_limit, upper_limit);
  }

  char *output_filename = malloc(sizeof(char) * 256);
  sprintf(output_filename, "Resultados/command%d_output.txt", counter);

  FILE *output_file = fopen(output_filename, "w");

  if (output_file == NULL) {
    printf("Error creating command%d_output.txt file\n", counter);
    return;
  }

  if (average) {
    fprintf(output_file, "%.3f\n", average);
  }

  free(parameters);
  free(output_filename);
  fclose(output_file);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("%d: Query %d elapsed time: %f seconds\n", counter,
         5 + query6_determiner, time_spent);

  // Since catalog is not used in this query, we can safely ignore the warning
  (void)catalog;
}

double calculate_average_price(GHashTable *rides_by_date, int lower_limit,
                               int upper_limit) {
  double total = 0.f;
  int rides_counter = 0;
  int temp_date;

  for (temp_date = lower_limit; upper_limit >= temp_date;
       temp_date = increment_date(temp_date)) {
    RIDES_OF_THE_DAY rides_of_the_day =
        g_hash_table_lookup(rides_by_date, &temp_date);

    if (rides_of_the_day) {
      GArray *day_rides = get_ride_of_the_day_array(rides_of_the_day);

      // If this day's total price wasn't calculated yet, we calculate it

      if (get_ride_of_the_day_avg_price(rides_of_the_day) == -1) {
        int number_of_cities = day_rides->len, day_rides_counter = 0;
        double daily_total = 0.f;
        // We add the price of every ride in every city in that day

        for (int i = 0; i < number_of_cities; i++) {
          GArray *city_rides = g_array_index(day_rides, GArray *, i);
          // We check if any rides were made on that city on that day
          if (city_rides) {
            int number_of_rides = city_rides->len;

            for (int j = 0; j < number_of_rides; j++) {
              RIDE temp_ride = g_array_index(city_rides, RIDE, j);
              // Make sure there is something in that array position
              if (temp_ride == NULL) {
                break;
              }

              daily_total += get_ride_price(temp_ride);
              day_rides_counter++;
            }
          }
        }

        // If any rides were made in that day, we add them to the total
        if (day_rides_counter) {
          set_ride_of_the_day_avg_price(rides_of_the_day, daily_total);
          set_ride_of_the_day_number_of_rides(rides_of_the_day,
                                              day_rides_counter);
        } else {
          set_ride_of_the_day_avg_price(rides_of_the_day, 0);
          set_ride_of_the_day_number_of_rides(rides_of_the_day, 0);
        }
      }

      total += get_ride_of_the_day_avg_price(rides_of_the_day);
      rides_counter += get_ride_of_the_day_number_of_rides(rides_of_the_day);
    }
  }

  if (rides_counter) {
    return (double)(total / rides_counter);
  }
  return 0;
}

double calculate_average_distance(GHashTable *rides_by_date, int lower_limit,
                                  int upper_limit, char city_code) {
  double total = 0.f;
  int rides_counter = 0;
  int temp_date;
  RIDE temp_ride = NULL;

  for (temp_date = lower_limit; upper_limit >= temp_date;
       temp_date = increment_date(temp_date)) {
    RIDES_OF_THE_DAY rides_of_the_day =
        g_hash_table_lookup(rides_by_date, &temp_date);

    if (rides_of_the_day) {
      GArray *array = get_ride_of_the_day_array(rides_of_the_day);

      // Prevent checking NULL and out of bounds
      if (array && array->len >= (guint)city_code) {
        GArray *city_rides = g_array_index(array, GArray *, (int)city_code);
        if (city_rides) {
          int number_of_rides = city_rides->len;
          for (int i = 0; i < number_of_rides; i++) {
            temp_ride = g_array_index(city_rides, RIDE, i);
            // Make sure there is something in that array position
            if (temp_ride == NULL) {
              break;
            }
            total += get_ride_distance(temp_ride);
            rides_counter++;
          }
        }
      }
    }
  }

  if (rides_counter) {
    return (double)(total / rides_counter);
  }

  return 0;
}

void query7(CATALOG catalog, STATS stats, char **parameter, int counter) {
  clock_t begin = clock();

  char *output_filename = malloc(sizeof(char) * 256);
  sprintf(output_filename, "Resultados/command%d_output.txt", counter);

  FILE *output_file = fopen(output_filename, "a");

  if (output_file == NULL) {
    printf("Error creating command%d_output.txt file\n", counter);
    return;
  }

  int n;
  sscanf(parameter[0], "%d", &n);
  char *city_string = strip(parameter[1]);
  GHashTable *city_code = get_catalog_city_code(catalog);
  char *city_number = g_hash_table_lookup(city_code, city_string);

  if (city_number) {
    int city = *city_number;

    /* GTree *city_drivers_tree = get_city_stats_tree(stats, city); */
    GHashTable *city_drivers_hash = get_city_stats_hash(stats, city);

    if (city_drivers_hash == NULL) {
      return;
    }

    GPtrArray *city_drivers_array = get_city_stats_array(stats, city);
    g_ptr_array_sort(city_drivers_array, compare_driver_stats_by_average_score);

    /* if (city_drivers_array == NULL) { */
    /*   city_drivers_array = g_ptr_array_new(); */

    /*   g_tree_foreach(city_drivers_tree, (GTraverseFunc)tree_to_array, */
    /*                  city_drivers_array); */
    /*   g_ptr_array_sort(city_drivers_array, */
    /*                    compare_driver_stats_by_average_score); */

    /*   set_city_drivers_array(stats, city, city_drivers_array); */
    /* } */

    int i = 0;
    while (n > 0 && i < (int)city_drivers_array->len) {
      CITY_DRIVER_STATS city_driver_stats =
          g_ptr_array_index(city_drivers_array, i);
      int driver_id = get_city_driver_stats_id(city_driver_stats);

      GHashTable *drivers = get_catalog_drivers(catalog);
      DRIVER driver = g_hash_table_lookup(drivers, &driver_id);
      enum account_status status = get_driver_account_status(driver);

      if (status == ACTIVE) {
        char *driver_name = get_catalog_driver_name(catalog, &driver_id);

        double driver_rating =
            get_city_driver_stats_total_rating(city_driver_stats);

        int driver_number_of_rides =
            get_city_driver_stats_total_rides(city_driver_stats);

        double driver_average_score = driver_rating / driver_number_of_rides;

        fprintf(output_file, "%012d;%s;%.3f\n", driver_id, driver_name,
                driver_average_score);

        free(driver_name);
        n--;
      }

      i++;
    }
  }

  free(city_string);
  free(parameter);
  free(output_filename);
  fclose(output_file);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("%d: Query 7 elapsed time: %f seconds\n", counter, time_spent);
}

void query8(CATALOG catalog, STATS stats, char **parameter, int counter) {
  clock_t begin = clock();

  char *gender = parameter[0];
  enum gender given_gender = string_to_gender(gender);

  int age;
  sscanf(parameter[1], "%d", &age);

  char *output_filename = malloc(sizeof(char) * 256);
  sprintf(output_filename, "Resultados/command%d_output.txt", counter);

  FILE *output_file = fopen(output_filename, "a");

  if (output_file == NULL) {
    printf("Error creating command%d_output.txt file\n", counter);
    return;
  }

  static int males_sorted = 0;
  static int females_sorted = 0;

  GArray *top_rides = NULL;

  if (given_gender == M) {
    top_rides = get_male_rides_by_age(stats);

    if (!males_sorted) {
      qsort(top_rides->data, top_rides->len, sizeof(RIDE_GENDER_STATS),
            (GCompareFunc)compare_rides_by_age);
      top_rides = get_male_rides_by_age(stats);

      males_sorted = 1;
    }
  } else {
    top_rides = get_female_rides_by_age(stats);

    if (!females_sorted) {
      qsort(top_rides->data, top_rides->len, sizeof(RIDE_GENDER_STATS),
            (GCompareFunc)compare_rides_by_age);
      top_rides = get_female_rides_by_age(stats);

      females_sorted = 1;
    }
  }

  GPtrArray *users_reverse_lookup = get_catalog_users_reverse_lookup(catalog);

  for (int i = top_rides->len - 1; i >= 0; i--) {
    RIDE_GENDER_STATS current_ride =
        g_array_index(top_rides, RIDE_GENDER_STATS, i);
    int ride_id = get_ride_gender_stats_id(current_ride);

    int driver_age = calculate_age(
        get_ride_gender_stats_driver_account_creation(current_ride));

    if (driver_age < age) {
      break;
    }

    int user_age = calculate_age(
        get_ride_gender_stats_user_account_creation(current_ride));

    if (user_age < age) {
      continue;
    }

    GHashTable *rides = get_catalog_rides(catalog);
    RIDE current_ride_catalog = g_hash_table_lookup(rides, &ride_id);

    int username = get_ride_user(current_ride_catalog);
    USER current_user =
        g_hash_table_lookup(get_catalog_users(catalog), &username);

    int driver_id = get_ride_driver(current_ride_catalog);
    DRIVER current_driver =
        g_hash_table_lookup(get_catalog_drivers(catalog), &driver_id);

    char *driver_name = get_driver_name(current_driver);
    char *name = get_user_name(current_user);

    char *username_string = g_ptr_array_index(users_reverse_lookup, username);
    fprintf(output_file, "%012d;%s;%s;%s\n", driver_id, driver_name,
            username_string, name);

    free(driver_name);
    free(name);
  }

  free(parameter);
  free(output_filename);
  fclose(output_file);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("%d: Query 8 elapsed time: %f seconds\n", counter, time_spent);

  (void)catalog;
  (void)stats;
  (void)parameter;
  (void)counter;
}

void query9(CATALOG catalog, STATS stats, char **parameter, int counter) {
  clock_t begin = clock();

  GHashTable *rides_by_date = get_rides_by_date(stats);

  // The array where rides will be ordered is created
  GArray *rides_in_range = g_array_new(1, 1, sizeof(RIDE));
  int lower_limit = date_string_to_int(parameter[0]);
  int upper_limit = date_string_to_int(parameter[1]);
  int number_of_rides = 0;

  for (int temp_date = lower_limit; upper_limit >= temp_date;
       temp_date = increment_date(temp_date)) {
    // Tries to find if there are rides in that specific day
    RIDES_OF_THE_DAY rides_of_the_day_struct =
        g_hash_table_lookup(rides_by_date, &temp_date);
    if (rides_of_the_day_struct) {
      GArray *rides_of_the_day =
          get_ride_of_the_day_array(rides_of_the_day_struct);

      if (rides_of_the_day) {
        int number_of_cities = rides_of_the_day->len;

        for (int i = 0; i < number_of_cities; i++) {
          GArray *city_rides = g_array_index(rides_of_the_day, GArray *, i);

          if (city_rides) {
            int number_of_rides = city_rides->len;

            for (int j = 0; j < number_of_rides; j++) {
              RIDE temp_ride = g_array_index(city_rides, RIDE, j);
              // Make sure there is something in that array position
              if (temp_ride == NULL) break;
              // The ride is inserted into the temporary structure
              g_array_append_val(rides_in_range, temp_ride);
            }
          }
        }
      }
    }
  }

  qsort(rides_in_range->data, rides_in_range->len, sizeof(RIDE),
        (GCompareFunc)compare_rides_by_distance);

  char *output_filename = malloc(sizeof(char) * 256);
  sprintf(output_filename, "Resultados/command%d_output.txt", counter);

  FILE *output_file = fopen(output_filename, "a");

  if (output_file == NULL) {
    printf("Error creating command%d_output.txt file\n", counter);
    return;
  }

  GPtrArray *city_lookup_reverse = get_catalog_city_reverse_lookup(catalog);
  number_of_rides = rides_in_range->len - 1;
  while (number_of_rides >= 0) {
    RIDE temp_ride = g_array_index(rides_in_range, RIDE, number_of_rides);
    char *temp_date = date_to_string(get_ride_date(temp_ride));
    fprintf(output_file, "%012d;%s;%d;%s;%.3f\n", get_ride_id(temp_ride),
            temp_date, get_ride_distance(temp_ride),
            (char *)(g_ptr_array_index(city_lookup_reverse,
                                       get_ride_city(temp_ride))),
            get_ride_tip(temp_ride));
    number_of_rides--;
    free(temp_date);
  }

  free(parameter);
  free(output_filename);
  fclose(output_file);
  g_array_free(rides_in_range, 1);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("%d: Query 9 elapsed time: %f seconds\n", counter, time_spent);

  (void)catalog;
  (void)stats;
  (void)parameter;
  (void)counter;
}

gint compare_rides_by_distance(gconstpointer a, gconstpointer b) {
  int temp = 0;
  RIDE ride1 = *(RIDE *)(a);
  RIDE ride2 = *(RIDE *)(b);

  int ride1_distance = get_ride_distance(ride1);
  int ride2_distance = get_ride_distance(ride2);
  temp = ride1_distance - ride2_distance;
  if (temp) return temp;

  int ride1_date = get_ride_date(ride1);
  int ride2_date = get_ride_date(ride2);
  temp = ride1_date - ride2_date;
  if (temp) return temp;

  int ride1_id = get_ride_id(ride1);
  int ride2_id = get_ride_id(ride2);

  return ride1_id - ride2_id;
}

void get_user_profile(CATALOG catalog, char *id, int counter) {
  char *output_filename = malloc(sizeof(char) * 256);
  sprintf(output_filename, "Resultados/command%d_output.txt", counter);

  FILE *output_file = fopen(output_filename, "w");

  if (output_file == NULL) {
    printf("Error creating command%d_output.txt file\n", counter);
    return;
  }

  GHashTable *users_code = get_catalog_users_code(catalog);
  int *user_code = g_hash_table_lookup(users_code, id);

  if (user_code == NULL) {
    free(output_filename);
    fclose(output_file);
    printf("User with id %s does not exist\n", id);
    return;
  }

  GHashTable *users_hash_table = get_catalog_users(catalog);
  USER user = g_hash_table_lookup(users_hash_table, user_code);

  char *name = get_user_name(user);
  enum gender gender = get_user_gender(user);

  int number_of_rides = get_user_number_of_rides(user);
  double average_rating =
      (double)(get_user_total_rating(user) / number_of_rides);

  double total_spent = get_user_total_spent(user);
  enum account_status account_status = get_user_account_status(user);

  if (account_status == ACTIVE) {
    fprintf(output_file, "%s;%s;%d;%.3f;%d;%.3f\n", name,
            gender_to_string(gender), calculate_age(get_user_birth_date(user)),
            average_rating, number_of_rides, total_spent);
  }

  free(name);
  free(output_filename);
  fclose(output_file);
}

void get_driver_profile(CATALOG catalog, char *id, int counter) {
  char *output_filename = malloc(sizeof(char) * 256);
  sprintf(output_filename, "Resultados/command%d_output.txt", counter);
  FILE *output_file = fopen(output_filename, "w");

  if (output_file == NULL) {
    printf("Error creating command%d_output.txt file\n", counter);
    return;
  }

  GHashTable *drivers_hash_table = get_catalog_drivers(catalog);
  int id_int = atoi(id);
  DRIVER driver = g_hash_table_lookup(drivers_hash_table, &id_int);

  if (driver == NULL) {
    free(output_filename);
    fclose(output_file);
    printf("Driver with id %s does not exist\n", id);
    return;
  }

  char *name = get_driver_name(driver);
  enum gender gender = get_driver_gender(driver);

  int number_of_rides = get_driver_number_of_rides(driver);
  double average_rating =
      (double)(get_driver_total_rating(driver) / number_of_rides);

  double total_earned = get_driver_total_earned(driver);
  enum account_status account_status = get_driver_account_status(driver);

  if (account_status == ACTIVE) {
    fprintf(output_file, "%s;%s;%d;%.3f;%d;%.3f\n", name,
            gender_to_string(gender),
            calculate_age(get_driver_birth_date(driver)), average_rating,
            number_of_rides, total_earned);
  }

  free(name);
  free(output_filename);
  fclose(output_file);
}
