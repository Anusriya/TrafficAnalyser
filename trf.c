#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

typedef struct TrafficData {
    int traffic;
    time_t timestamp;
    struct TrafficData* next;
    struct TrafficData* prev;
} TrafficData;

void add_traffic_data(TrafficData** head, int traffic, time_t timestamp);
void read_traffic_data_from_csv(const char* filename, TrafficData** head);
void print_traffic_data_in_time_range(TrafficData* head, time_t input_time, int window_seconds, int direction);
void print_average_traffic_in_time_range(TrafficData* head, time_t input_time, int window_seconds, int direction);
void add_traffic_data_to_file(TrafficData** head, const char* filename);
void delete_traffic_data_from_file(TrafficData** head, const char* filename);
void find_peak_traffic_hour(TrafficData* head);
void display_menu();
void free_linked_list(TrafficData* head);

void export_data_to_csv(TrafficData* head, const char* filename);
void display_traffic_summary(TrafficData* head);

void add_traffic_data(TrafficData** head, int traffic, time_t timestamp) {
    TrafficData* new_data = malloc(sizeof(TrafficData));
    new_data->traffic = traffic;
    new_data->timestamp = timestamp;
    new_data->next = *head;
    new_data->prev = NULL;

    if (*head != NULL) {
        (*head)->prev = new_data; 
    }
    *head = new_data;
}
void read_traffic_data_from_csv(const char* filename, TrafficData** head) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char timestamp_str[30];
        int traffic;
        if (sscanf(line, "%d, %[^\n]", &traffic, timestamp_str) == 2) {
            struct tm t = {0}; 
            if (sscanf(timestamp_str, "%*s %*s %d %d:%d:%d %d",
                       &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &t.tm_year) == 5) {
                t.tm_year -= 1900; 
                t.tm_mon = 8; 
                time_t ts = mktime(&t);
                add_traffic_data(head, traffic, ts);
            } else {
                printf("Error parsing timestamp: %s\n", timestamp_str);
            }
        } else {
            printf("Error reading line: %s\n", line);
        }
    }

    fclose(file);
}

void find_peak_traffic_hour(TrafficData* head) {
    if (head == NULL) {
        printf("No traffic data available.\n");
        return;
    }

    char start_time_str[30], end_time_str[30];
    printf("Enter the start time (e.g., 'Fri Sep 29 00:00:00 2023'): ");
    scanf(" %[^\n]", start_time_str);

    printf("Enter the end time (e.g., 'Fri Sep 29 23:59:59 2023'): ");
    scanf(" %[^\n]", end_time_str);

    struct tm start_tm = {0}, end_tm = {0};

    sscanf(start_time_str, "%*s %*s %d %d:%d:%d %d",
           &start_tm.tm_mday, &start_tm.tm_hour, &start_tm.tm_min, &start_tm.tm_sec, &start_tm.tm_year);
    start_tm.tm_year -= 1900; 
    start_tm.tm_mon = 8; 
    time_t start_time = mktime(&start_tm);

    sscanf(end_time_str, "%*s %*s %d %d:%d:%d %d",
           &end_tm.tm_mday, &end_tm.tm_hour, &end_tm.tm_min, &end_tm.tm_sec, &end_tm.tm_year);
    end_tm.tm_year -= 1900; 
    end_tm.tm_mon = 8; 
    time_t end_time = mktime(&end_tm);

    if (difftime(end_time, start_time) <= 0) {
        printf("Invalid time range. The end time must be after the start time.\n");
        return;
    }

    int traffic_per_hour[24] = {0};
    TrafficData* current = head;
    while (current != NULL) {
        if (current->timestamp >= start_time && current->timestamp <= end_time) {
            struct tm* tm_info = localtime(&current->timestamp);
            int hour = tm_info->tm_hour;
            traffic_per_hour[hour] += current->traffic;
        }
        current = current->next;
    }

   
    int peak_hour = -1;
    int max_traffic = -1;
    for (int i = 0; i < 24; i++) {
        if (traffic_per_hour[i] > max_traffic) {
            max_traffic = traffic_per_hour[i];
            peak_hour = i;
        }
    }

    if (max_traffic > 0) {
        printf("Peak traffic hour: %02d:00 with total traffic of %d within the given range\n", peak_hour, max_traffic);
    } else {
        printf("No traffic data found within the specified time range.\n");
    }
}

void add_traffic_data_to_file(TrafficData** head, const char* filename) {
    int traffic;
    char timestamp_str[30];

    
    printf("Enter traffic value: ");
    scanf("%d", &traffic);

    printf("Enter the time (e.g., 'Fri Sep 29 20:05:00 2023'): ");
    scanf(" %[^\n]", timestamp_str);

    struct tm t = {0};
    sscanf(timestamp_str, "%*s %*s %d %d:%d:%d %d",
           &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &t.tm_year);
    t.tm_year -= 1900;
    t.tm_mon = 8; 
    time_t timestamp = mktime(&t);

    
    add_traffic_data(head, traffic, timestamp);

    
    FILE* file = fopen(filename, "a");
    if (!file) {
        perror("Error opening file");
        return;
    }

    
    fprintf(file, "\n%d, %s", traffic, timestamp_str);  
    fclose(file);

    printf("Traffic data added successfully.\n");
}

void print_traffic_data_in_time_range(TrafficData* head, time_t input_time, int window_seconds, int direction) {
    time_t start_time, end_time;
    if (direction == 1) {
        start_time = input_time;
    end_time = input_time + window_seconds;
    } else {
    end_time = input_time;
        start_time = input_time - window_seconds;
    }

    printf("Traffic values within the sliding window:\n");
    printf("---------------------------------------------------\n");
    TrafficData* current = head;
    while (current != NULL) {
        if (current->timestamp >= start_time && current->timestamp <= end_time) {
            struct tm *tm_info = localtime(&current->timestamp);
            char buffer[30];
            strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);
            printf("Traffic: %d, Timestamp: %s\n", current->traffic, buffer);
        }
        current = current->next;
    }
}

void delete_traffic_data_from_file(TrafficData** head, const char* filename) {
    char timestamp_str[30];
    printf("Enter the timestamp of the traffic data to delete (e.g., 'Fri Sep 29 20:05:00 2023'): ");
    scanf(" %[^\n]", timestamp_str);

    struct tm t = {0};
    sscanf(timestamp_str, "%*s %*s %d %d:%d:%d %d",
           &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &t.tm_year);
    t.tm_year -= 1900;
    t.tm_mon = 8; 
    time_t timestamp = mktime(&t);

    TrafficData* current = *head;
    while (current != NULL && difftime(current->timestamp, timestamp) != 0) {
        current = current->next;
    }

    
    if (current != NULL) {
        if (current->prev != NULL) {
            current->prev->next = current->next;
        } else {
           
            *head = current->next;
        }
        if (current->next != NULL) {
            current->next->prev = current->prev;
        }

        free(current); 

        
        FILE* file = fopen(filename, "w");
        if (!file) {
            perror("Error opening file");
            return;
        }

        TrafficData* temp = *head;
        while (temp != NULL) {
            struct tm* tm_info = localtime(&temp->timestamp);
            char buffer[30];
            strftime(buffer, 30, "%a %b %d %H:%M:%S %Y", tm_info);
            fprintf(file, "%d, %s\n", temp->traffic, buffer);
            temp = temp->next;
        }

        fclose(file);
        printf("Traffic data deleted successfully.\n");
    } else {
        printf("No traffic data found for the specified timestamp.\n");
    }
}

void print_average_traffic_in_time_range(TrafficData* head, time_t input_time, int window_seconds, int direction) {
    time_t start_time, end_time;
    if (direction == 1) {
        start_time = input_time;
    end_time = input_time + window_seconds;
    } else {
        start_time = input_time - window_seconds;
    end_time = input_time;

    }

    int total_traffic = 0;
    int count = 0;

    printf("Traffic values within the sliding window for average:\n");
    printf("---------------------------------------------------\n");
    TrafficData* current = head;
    while (current != NULL) {
        if (current->timestamp >= start_time && current->timestamp <= end_time) {
            struct tm *tm_info = localtime(&current->timestamp);
            char buffer[30];
            strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);
            printf("Traffic: %d, Timestamp: %s\n", current->traffic, buffer);
            total_traffic += current->traffic;
            count++;
        }
        current = current->next;
    }

    if (count > 0) {
        double average_traffic = (double)total_traffic / count;
        printf("Average Traffic: %.2f\n", average_traffic);
    } else {
        printf("No traffic data found in the specified range.\n");
    }
}

void display_menu() {
    printf("\n");
    printf("====================================\n");
    printf("        TRAFFIC ANALYZER MENU        \n");
    printf("====================================\n");
    printf("| 1. View Traffic Data in Time Range |\n");
    printf("| 2. Analyze Average Traffic         |\n");
    printf("| 3. Find Peak Traffic Hour          |\n");
    printf("| 4. Add Traffic Data                |\n");
    printf("| 5. Delete Traffic Data             |\n");
    printf("| 6. Export Traffic Data to CSV      |\n");
    printf("| 7. Display Traffic Summary         |\n");
    printf("| 8. Visualise Traffic Data          |\n");
    printf("| 9. Exit                            |\n");
    printf("====================================\n");
    printf("Please select an option: ");
}

void export_data_to_csv(TrafficData* head, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for export");
        return;
    }

    fprintf(file, "Traffic, Timestamp\n");
    TrafficData* current = head;
    while (current != NULL) {
        struct tm* tm_info = localtime(&current->timestamp);
        char buffer[30];
        strftime(buffer, 30, "%a %b %d %H:%M:%S %Y", tm_info);
        fprintf(file, "%d, %s\n", current->traffic, buffer);
        current = current->next;
    }

    fclose(file);
    printf("Traffic data exported successfully to %s.\n", filename);
}
void free_linked_list(TrafficData* head) {
    TrafficData* current = head;
    while (current != NULL) {
        TrafficData* next = current->next;
        free(current);
        current = next;
    }
}

void write_traffic_data_for_gnuplot(TrafficData* head, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        return;
    }

    TrafficData* current = head;
    while (current != NULL) {
        struct tm* tm_info = localtime(&current->timestamp);
        char time_str[30];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        
        fprintf(file, "%s %d\n", time_str, current->traffic);
        current = current->next;
    }

    fclose(file);
}

void plot_traffic_data_with_gnuplot(const char* filename) {
    FILE* gnuplot = popen("gnuplot -persistent", "w");
    if (gnuplot == NULL) {
        perror("Error starting gnuplot");
        return;
    }

    
    fprintf(gnuplot, "set terminal qt size 800,600\n"); 
    fprintf(gnuplot, "set title 'Traffic Analysis'\n");
    fprintf(gnuplot, "set xlabel 'Time'\n");
    fprintf(gnuplot, "set ylabel 'Traffic Volume'\n");

    fprintf(gnuplot, "set xdata time\n");
    fprintf(gnuplot, "set timefmt '%%Y-%%m-%%d %%H:%%M:%%S'\n");
    fprintf(gnuplot, "set format x '%%H:%%M'\n");

    
    fprintf(gnuplot, "set autoscale\n");
    fprintf(gnuplot, "set xtics auto\n"); 
    fprintf(gnuplot, "set ytics auto\n"); 
    fprintf(gnuplot, "set xlabel 'Time' font ',10'\n"); 
    fprintf(gnuplot, "set ylabel 'Traffic Volume' font ',10'\n"); 
    fprintf(gnuplot, "set xtics font ',8'\n"); 
    fprintf(gnuplot, "set ytics font ',8'\n"); 
    fprintf(gnuplot, "set grid\n");

    
    fprintf(gnuplot, "plot '%s' using 1:2 with lines title 'Traffic'\n", filename);


    fprintf(gnuplot, "pause -1\n");
    pclose(gnuplot);
}


void display_traffic_summary(TrafficData* head) {
    if (head == NULL) {
        printf("No traffic data available.\n");
        return;
    }

    int total_entries = 0;
    int total_traffic = 0;
    int max_traffic = -1;
    int min_traffic = INT_MAX;

    TrafficData* current = head;
    while (current != NULL) {
        total_entries++;
        total_traffic += current->traffic;
        if (current->traffic > max_traffic) {
            max_traffic = current->traffic;
        }
        if (current->traffic < min_traffic) {
            min_traffic = current->traffic;
        }
        current = current->next;
    }

    printf("Traffic Summary:\n");
    printf("---------------------------------------------------\n");
    printf("Total Entries: %d\n", total_entries);
    printf("Total Traffic: %d\n", total_traffic);
    printf("Average Traffic: %.2f\n", (double)total_traffic / total_entries);
    printf("Maximum Traffic: %d\n", max_traffic);
    printf("Minimum Traffic: %d\n", min_traffic);
}


int main() {
    TrafficData* traffic_data = NULL; 
    read_traffic_data_from_csv("trafficdata.csv", &traffic_data);

    int choice;
    do {
        display_menu();

        scanf("%d", &choice);

        if (choice == 1) {
            
            char timestamp_str[30];
            int window_seconds, direction;

            printf("Enter the time (e.g., 'Fri Sep 29 20:05:00 2023'): ");
            scanf(" %[^\n]", timestamp_str);

            struct tm t = {0};
            sscanf(timestamp_str, "%*s %*s %d %d:%d:%d %d",
                   &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &t.tm_year);
            t.tm_year -= 1900;
            t.tm_mon = 8; 
            time_t timestamp = mktime(&t);

            printf("Enter the time window in seconds: ");
            scanf("%d", &window_seconds);

            printf("Direction (1 = forward, -1 = backward): ");
            scanf("%d", &direction);

            print_traffic_data_in_time_range(traffic_data, timestamp, window_seconds, direction);

        } else if (choice == 2) {
            
            char timestamp_str[30];
            int window_seconds, direction;

            printf("Enter the time (e.g., 'Fri Sep 29 20:05:00 2023'): ");
            scanf(" %[^\n]", timestamp_str);

            struct tm t = {0};
            sscanf(timestamp_str, "%*s %*s %d %d:%d:%d %d",
                   &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &t.tm_year);
            t.tm_year -= 1900;
            t.tm_mon = 8; 
            time_t timestamp = mktime(&t);

            printf("Enter the time window in seconds: ");
            scanf("%d", &window_seconds);

            printf("Direction (1 = forward, -1 = backward): ");
            scanf("%d", &direction);

            print_average_traffic_in_time_range(traffic_data, timestamp, window_seconds, direction);

        } else if (choice == 3) {
            find_peak_traffic_hour(traffic_data);
        } else if (choice == 4) {
            add_traffic_data_to_file(&traffic_data, "trafficdata.csv");
        } else if (choice == 5) {
            delete_traffic_data_from_file(&traffic_data, "trafficdata.csv");
        } else if (choice == 6) {
            export_data_to_csv(traffic_data, "exported_trafficdata.csv");
        } else if (choice == 7) {
            display_traffic_summary(traffic_data);
        } else if (choice == 8) {
            write_traffic_data_for_gnuplot(traffic_data, "traffic_data.txt");
            plot_traffic_data_with_gnuplot("traffic_data.txt");
        } else if (choice == 9) {
            printf("Exiting...\n");
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 9);

    free_linked_list(traffic_data);
    return 0;
}
