#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structures
typedef struct {
    int size;
    double *data;
} Grades;

typedef struct {
    double coefficient;
    double average;
    char *name;
    Grades notes;
} Course;

typedef struct {
    int id;
    int age;
    int courses_count;
    double overall_average;
    char *first_name;
    char *last_name;
    Course **courses;
} Student;

typedef struct {
    Student **students;
    int students_count;
    Course **courses;
    int courses_count;
} Class;

typedef struct {
    Student *student;
    double average;
} StudentRanking;





int est_ligne_vide(char *ligne) {
    if (ligne == NULL) return 1;
    while (*ligne) {
        if (!isspace((unsigned char)*ligne)) return 0;
        ligne++;
    }
    return 1;
}


void supprimer_newline(char *ligne) {
    if (ligne == NULL) return;
    size_t len = strlen(ligne);
    if (len > 0 && ligne[len-1] == '\n') {
        ligne[len-1] = '\0';
    }
    if (len > 1 && ligne[len-2] == '\r') {
        ligne[len-2] = '\0';
    }
}


int est_entete_etudiant(char *ligne) {
    return (strstr(ligne, "numero") != NULL && 
            strstr(ligne, "prenom") != NULL && 
            strstr(ligne, "nom") != NULL);
}


int est_entete_matiere(char *ligne) {
    return (strstr(ligne, "nom") != NULL && 
            strstr(ligne, "coef") != NULL &&
            strstr(ligne, "prenom") == NULL);
}


int est_entete_note(char *ligne) {
    return (strstr(ligne, "id") != NULL && 
            strstr(ligne, "nom") != NULL && 
            strstr(ligne, "note") != NULL);
}


// FONCTIONS POUR GRADES



void init_grades(Grades *grades) {
    if (grades == NULL) return;
    grades->data = NULL;
    grades->size = 0;
}


int add_grade(Grades *grades, double grade) {
    if (grades == NULL) return -1;
    
    double *new_data = (double*)realloc(grades->data, (grades->size + 1) * sizeof(double));
    if (new_data == NULL) return -1;
    
    grades->data = new_data;
    grades->data[grades->size] = grade;
    grades->size++;
    
    return 0;
}


double calculate_grades_average(const Grades *grades) {
    if (grades == NULL || grades->size == 0) return 0.0;
    
    double sum = 0.0;
    for (int i = 0; i < grades->size; i++) {
        sum += grades->data[i];
    }
    
    return sum / grades->size;
}


void free_grades(Grades *grades) {
    if (grades == NULL) return;
    
    if (grades->data != NULL) {
        free(grades->data);
        grades->data = NULL;
    }
    grades->size = 0;
}


// FONCTIONS POUR COURSE



Course* create_course(const char *name, double coefficient) {
    if (name == NULL) return NULL;
    
    Course *course = (Course*)malloc(sizeof(Course));
    if (course == NULL) return NULL;
    
    // Allocation au plus juste pour le nom
    course->name = (char*)malloc((strlen(name) + 1) * sizeof(char));
    if (course->name == NULL) {
        free(course);
        return NULL;
    }
    strcpy(course->name, name);
    
    course->coefficient = coefficient;
    course->average = 0.0;
    init_grades(&course->notes);
    
    return course;
}


int add_grade_to_course(Course *course, double grade) {
    if (course == NULL) return -1;
    
    if (add_grade(&course->notes, grade) != 0) return -1;
    
    // Mise à jour de la moyenne
    course->average = calculate_grades_average(&course->notes);
    
    return 0;
}


void free_course(Course *course) {
    if (course == NULL) return;
    
    if (course->name != NULL) {
        free(course->name);
        course->name = NULL;
    }
    
    free_grades(&course->notes);
    free(course);
}

// FONCTIONS POUR STUDENT



Student* create_student(int id, const char *first_name, const char *last_name, int age) {
    if (first_name == NULL || last_name == NULL) return NULL;
    
    Student *student = (Student*)malloc(sizeof(Student));
    if (student == NULL) return NULL;
    
    // Allocation au plus juste pour le prénom
    student->first_name = (char*)malloc((strlen(first_name) + 1) * sizeof(char));
    if (student->first_name == NULL) {
        free(student);
        return NULL;
    }
    strcpy(student->first_name, first_name);
    
    // Allocation au plus juste pour le nom
    student->last_name = (char*)malloc((strlen(last_name) + 1) * sizeof(char));
    if (student->last_name == NULL) {
        free(student->first_name);
        free(student);
        return NULL;
    }
    strcpy(student->last_name, last_name);
    
    student->id = id;
    student->age = age;
    student->courses = NULL;
    student->courses_count = 0;
    student->overall_average = 0.0;
    
    return student;
}


Course* find_or_create_course(Student *student, const char *course_name, double coefficient) {
    if (student == NULL || course_name == NULL) return NULL;
    
    // Chercher si la matière existe déjà
    for (int i = 0; i < student->courses_count; i++) {
        if (strcmp(student->courses[i]->name, course_name) == 0) {
            return student->courses[i];
        }
    }
    
    // Créer une nouvelle matière
    Course *new_course = create_course(course_name, coefficient);
    if (new_course == NULL) return NULL;
    
    // Agrandir le tableau de matières
    Course **new_courses = (Course**)realloc(student->courses, 
                                             (student->courses_count + 1) * sizeof(Course*));
    if (new_courses == NULL) {
        free_course(new_course);
        return NULL;
    }
    
    student->courses = new_courses;
    student->courses[student->courses_count] = new_course;
    student->courses_count++;
    
    return new_course;
}


void calculer_moyenne_generale(Student *student) {
    if (student == NULL) return;
    
    double sum_weighted = 0.0;
    double sum_coefficients = 0.0;
    
    for (int i = 0; i < student->courses_count; i++) {
        Course *course = student->courses[i];
        if (course->notes.size > 0) {
            sum_weighted += course->average * course->coefficient;
            sum_coefficients += course->coefficient;
        }
    }
    
    if (sum_coefficients > 0) {
        student->overall_average = sum_weighted / sum_coefficients;
    } else {
        student->overall_average = 0.0;
    }
}


int add_grade_to_student(Student *student, const char *course_name, 
                         double coefficient, double grade) {
    if (student == NULL) return -1;
    
    Course *course = find_or_create_course(student, course_name, coefficient);
    if (course == NULL) return -1;
    
    if (add_grade_to_course(course, grade) != 0) return -1;
    
    // Mise à jour de la moyenne générale
    calculer_moyenne_generale(student);
    
    return 0;
}


void free_student(Student *student) {
    if (student == NULL) return;
    
    if (student->first_name != NULL) {
        free(student->first_name);
        student->first_name = NULL;
    }
    
    if (student->last_name != NULL) {
        free(student->last_name);
        student->last_name = NULL;
    }
    
    if (student->courses != NULL) {
        for (int i = 0; i < student->courses_count; i++) {
            free_course(student->courses[i]);
        }
        free(student->courses);
        student->courses = NULL;
    }
    
    free(student);
}


// FONCTIONS  CLASS



Class* create_class() {
    Class *class = (Class*)malloc(sizeof(Class));
    if (class == NULL) return NULL;
    
    class->students = NULL;
    class->students_count = 0;
    class->courses = NULL;
    class->courses_count = 0;
    
    return class;
}


int add_student_to_class(Class *class, Student *student) {
    if (class == NULL || student == NULL) return -1;
    
    Student **new_students = (Student**)realloc(class->students, 
                                                (class->students_count + 1) * sizeof(Student*));
    if (new_students == NULL) return -1;
    
    class->students = new_students;
    class->students[class->students_count] = student;
    class->students_count++;
    
    return 0;
}


Student* trouver_etudiant(Student **students, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (students[i]->id == id) {
            return students[i];
        }
    }
    return NULL;
}


void free_class(Class *class) {
    if (class == NULL) return;
    
    if (class->students != NULL) {
        for (int i = 0; i < class->students_count; i++) {
            free_student(class->students[i]);
        }
        free(class->students);
        class->students = NULL;
    }
    
    if (class->courses != NULL) {
        for (int i = 0; i < class->courses_count; i++) {
            free_course(class->courses[i]);
        }
        free(class->courses);
        class->courses = NULL;
    }
    
    free(class);
}


Student* parser_ligne_etudiant(char *ligne) {
    if (ligne == NULL || est_ligne_vide(ligne)) {
        return NULL;
    }
    
    supprimer_newline(ligne);
    
    // Vérifier si c'est une ligne d'en-tête
    if (est_entete_etudiant(ligne)) {
        return NULL;
    }
    
    int id, age;
    char first_name_temp[256];
    char last_name_temp[256];
    
    int nb_fields = sscanf(ligne, "%d;%255[^;];%255[^;];%d",
                           &id, first_name_temp, last_name_temp, &age);
    
    if (nb_fields != 4) {
        fprintf(stderr, "Error: invalid student line format: %s\n", ligne);
        return NULL;
    }
    
    return create_student(id, first_name_temp, last_name_temp, age);
}


Course* parser_ligne_matiere(char *ligne) {
    if (ligne == NULL || est_ligne_vide(ligne)) {
        return NULL;
    }
    
    supprimer_newline(ligne);
    
    // Vérifier si c'est une ligne d'en-tête
    if (est_entete_matiere(ligne)) {
        return NULL;
    }
    
    char name_temp[256];
    double coefficient;
    
    int nb_fields = sscanf(ligne, "%255[^;];%lf", name_temp, &coefficient);
    
    if (nb_fields != 2) {
        fprintf(stderr, "Error: invalid course line format: %s\n", ligne);
        return NULL;
    }
    
    return create_course(name_temp, coefficient);
}


int parser_ligne_note(char *ligne, Student **students, int students_count, 
                      Course **courses, int courses_count) {
    if (ligne == NULL || est_ligne_vide(ligne)) {
        return 0;
    }
    
    supprimer_newline(ligne);
    
    // Vérifier si c'est une ligne d'en-tête
    if (est_entete_note(ligne)) {
        return 0;
    }
    
    int student_id;
    char course_name[256];
    double grade;
    
    int nb_fields = sscanf(ligne, "%d;%255[^;];%lf", &student_id, course_name, &grade);
    
    if (nb_fields != 3) {
        fprintf(stderr, "Error: invalid grade line format: %s\n", ligne);
        return -1;
    }
    
    // Trouver l'étudiant
    Student *student = trouver_etudiant(students, students_count, student_id);
    if (student == NULL) {
        fprintf(stderr, "Warning: student with id %d not found\n", student_id);
        return -1;
    }
    
    // Trouver le coefficient de la matière
    double coefficient = 1.0;
    for (int i = 0; i < courses_count; i++) {
        if (strcmp(courses[i]->name, course_name) == 0) {
            coefficient = courses[i]->coefficient;
            break;
        }
    }
    
    return add_grade_to_student(student, course_name, coefficient, grade);
}





Class* load_class_from_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: cannot open file %s\n", filepath);
        return NULL;
    }
    
    Class *class = create_class();
    if (class == NULL) {
        fclose(file);
        return NULL;
    }
    
    char ligne[1024];
    enum { INIT, STUDENTS, COURSES, GRADES } state = INIT;
    
    while (fgets(ligne, sizeof(ligne), file) != NULL) {
        // Ignorer les lignes vides
        if (est_ligne_vide(ligne)) {
            continue;
        }
        
        supprimer_newline(ligne);
        
        // Détection des sections
        if (strstr(ligne, "ETUDIANTS") != NULL) {
            state = STUDENTS;
            continue;
        } else if (strstr(ligne, "MATIERES") != NULL) {
            state = COURSES;
            continue;
        } else if (strstr(ligne, "NOTES") != NULL) {
            state = GRADES;
            continue;
        }
        
        // Traitement selon l'état
        switch (state) {
            case STUDENTS: {
                Student *student = parser_ligne_etudiant(ligne);
                if (student != NULL) {
                    if (add_student_to_class(class, student) != 0) {
                        fprintf(stderr, "Error: failed to add student\n");
                        free_student(student);
                    }
                }
                break;
            }
            
            case COURSES: {
                Course *course = parser_ligne_matiere(ligne);
                if (course != NULL) {
                    Course **new_courses = (Course**)realloc(class->courses,
                                                            (class->courses_count + 1) * sizeof(Course*));
                    if (new_courses == NULL) {
                        free_course(course);
                        break;
                    }
                    class->courses = new_courses;
                    class->courses[class->courses_count] = course;
                    class->courses_count++;
                }
                break;
            }
            
            case GRADES: {
                parser_ligne_note(ligne, class->students, class->students_count,
                                 class->courses, class->courses_count);
                break;
            }
            
            default:
                break;
        }
    }
    
    fclose(file);
    
    printf("Loaded: %d students, %d courses\n", class->students_count, class->courses_count);
    
    return class;
}


// FONCTIONS D'AFFICHAGE



void display_grades(Grades *grades) {
    if (grades == NULL || grades->data == NULL || grades->size == 0) {
        printf("No grades");
        return;
    }
    
    printf("[");
    for (int i = 0; i < grades->size; i++) {
        printf("%.2f", grades->data[i]);
        if (i < grades->size - 1) {
            printf(", ");
        }
    }
    printf("]");
}


void display_course(Course *course) {
    if (course == NULL) return;
    
    printf("    - %s (Coefficient: %.2f)\n", course->name, course->coefficient);
    printf("      Grades: ");
    display_grades(&course->notes);
    
    if (course->notes.size > 0) {
        printf(" | Average: %.2f/20\n", course->average);
    } else {
        printf(" | No grades yet\n");
    }
}


void display_student(Student *student) {
    if (student == NULL) return;
    
    
    printf("Student ID: %d\n", student->id);
    printf("Name: %s %s\n", student->first_name, student->last_name);
    printf("Age: %d years old\n", student->age);
    
    if (student->courses_count == 0) {
        printf("No courses enrolled\n");
    } else {
        printf("Courses (%d):\n", student->courses_count);
        for (int i = 0; i < student->courses_count; i++) {
            display_course(student->courses[i]);
        }
        
        printf("Overall Average: %.2f/20\n", student->overall_average);
    }
    
   
}


void display_all_courses(Course **courses, int count) {
    if (courses == NULL || count == 0) {
        printf("No courses available\n");
        return;
    }
    
    printf("\n=== AVAILABLE COURSES ===\n");
    for (int i = 0; i < count; i++) {
        if (courses[i] != NULL) {
            printf("%2d. %-20s (Coef: %.2f)\n", 
                   i + 1, courses[i]->name, courses[i]->coefficient);
        }
    }
    
}


void display_students_summary(Class *class) {
    if (class == NULL || class->students == NULL) {
        printf("No students in class\n");
        return;
    }
    
    printf("\n=== STUDENTS SUMMARY ===\n");
    printf("%-10s | %-15s | %-15s | %5s | %8s\n", 
           "ID", "First Name", "Last Name", "Age", "Average");
    
    
    for (int i = 0; i < class->students_count; i++) {
        Student *s = class->students[i];
        if (s != NULL) {
            printf("%-10d | %-15s | %-15s | %5d | ", 
                   s->id, s->first_name, s->last_name, s->age);
            
            if (s->courses_count > 0 && s->overall_average > 0) {
                printf("%8.2f\n", s->overall_average);
            } else {
                printf("    N/A\n");
            }
        }
    }
    
}


void display_class(Class *class) {
    if (class == NULL) {
        printf("Error: Class is NULL\n");
        return;
    }
    
    printf("\n");
    
    
    // Afficher les statistiques générales
    printf("\nGeneral Statistics:\n");
    printf("  - Total Students: %d\n", class->students_count);
    printf("  - Total Courses: %d\n", class->courses_count);
    
    // Calculer et afficher la moyenne de la promotion
    if (class->students_count > 0) {
        double sum_averages = 0.0;
        int students_with_grades = 0;
        
        for (int i = 0; i < class->students_count; i++) {
            if (class->students[i]->courses_count > 0 && 
                class->students[i]->overall_average > 0) {
                sum_averages += class->students[i]->overall_average;
                students_with_grades++;
            }
        }
        
        if (students_with_grades > 0) {
            double class_average = sum_averages / students_with_grades;
            printf("  - Class Average: %.2f/20\n", class_average);
        }
    }
    
    // Afficher toutes les matières disponibles
    display_all_courses(class->courses, class->courses_count);
    
    // Afficher le résumé des étudiants
    display_students_summary(class);
    
    // Afficher les détails de chaque étudiant
    printf("\n");
   
    printf("DETAILED STUDENT LIST \n");
    
    
    for (int i = 0; i < class->students_count; i++) {
        display_student(class->students[i]);
    }
}


void display_student_by_id(Class *class, int student_id) {
    if (class == NULL) {
        printf("Error: Class is NULL\n");
        return;
    }
    
    Student *student = trouver_etudiant(class->students, class->students_count, student_id);
    
    if (student == NULL) {
        printf("Error: Student with ID %d not found\n", student_id);
        return;
    }
    
    display_student(student);
}


void display_course_statistics(Class *class, const char *course_name) {
    if (class == NULL || course_name == NULL) {
        printf("Error: Invalid parameters\n");
        return;
    }
    
    printf("\n COURSE STATISTICS \n");
    printf("Course: %s\n", course_name);
   
    
    int students_enrolled = 0;
    int total_grades = 0;
    double sum_averages = 0.0;
    double min_average = 20.0;
    double max_average = 0.0;
    Student *best_student = NULL;
    
    for (int i = 0; i < class->students_count; i++) {
        Student *student = class->students[i];
        
        for (int j = 0; j < student->courses_count; j++) {
            Course *course = student->courses[j];
            
            if (strcmp(course->name, course_name) == 0 && course->notes.size > 0) {
                students_enrolled++;
                total_grades += course->notes.size;
                sum_averages += course->average;
                
                if (course->average < min_average) {
                    min_average = course->average;
                }
                
                if (course->average > max_average) {
                    max_average = course->average;
                    best_student = student;
                }
                
                break;
            }
        }
    }
    
    if (students_enrolled == 0) {
        printf("No students enrolled in this course\n");
        
        return;
    }
    
    printf("Students enrolled: %d\n", students_enrolled);
    printf("Total grades: %d\n", total_grades);
    printf("Average grade: %.2f/20\n", sum_averages / students_enrolled);
    printf("Minimum average: %.2f/20\n", min_average);
    printf("Maximum average: %.2f/20\n", max_average);
    
    if (best_student != NULL) {
        printf("Best student: %s %s (%.2f/20)\n", 
               best_student->first_name, best_student->last_name, max_average);
    }
    
    
}


void display_students_below_threshold(Class *class, double threshold) {
    if (class == NULL) {
        printf("Error: Class is NULL\n");
        return;
    }
    
    
    printf("Threshold: %.2f/20\n", threshold);
    
    
    int count = 0;
    for (int i = 0; i < class->students_count; i++) {
        Student *s = class->students[i];
        if (s->courses_count > 0 && s->overall_average > 0 && 
            s->overall_average < threshold) {
            printf("%d. %s %s (ID: %d) - Average: %.2f/20\n",
                   ++count, s->first_name, s->last_name, s->id, s->overall_average);
        }
    }
    
    if (count == 0) {
        printf("No students found below threshold\n");
    } else {
        printf("\nTotal: %d student(s)\n", count);
    }
    
    
}




int save_string(FILE *file, const char *str) {
    if (str == NULL) {
        int len = 0;
        if (fwrite(&len, sizeof(int), 1, file) != 1) return -1;
        return 0;
    }
    
    int len = strlen(str) + 1;
    if (fwrite(&len, sizeof(int), 1, file) != 1) return -1;
    if (fwrite(str, sizeof(char), len, file) != len) return -1;
    
    return 0;
}


char* load_string(FILE *file) {
    int len;
    if (fread(&len, sizeof(int), 1, file) != 1) return NULL;
    
    if (len == 0) return NULL;
    
    char *str = (char*)malloc(len * sizeof(char));
    if (str == NULL) return NULL;
    
    if (fread(str, sizeof(char), len, file) != len) {
        free(str);
        return NULL;
    }
    
    return str;
}


int save_grades(FILE *file, Grades *grades) {
    if (grades == NULL) return -1;
    
    if (fwrite(&grades->size, sizeof(int), 1, file) != 1) return -1;
    
    if (grades->size > 0 && grades->data != NULL) {
        if (fwrite(grades->data, sizeof(double), grades->size, file) != grades->size) {
            return -1;
        }
    }
    
    return 0;
}


int load_grades(FILE *file, Grades *grades) {
    if (grades == NULL) return -1;
    
    if (fread(&grades->size, sizeof(int), 1, file) != 1) return -1;
    
    if (grades->size > 0) {
        grades->data = (double*)malloc(grades->size * sizeof(double));
        if (grades->data == NULL) return -1;
        
        if (fread(grades->data, sizeof(double), grades->size, file) != grades->size) {
            free(grades->data);
            return -1;
        }
    } else {
        grades->data = NULL;
    }
    
    return 0;
}


int save_course(FILE *file, Course *course) {
    if (course == NULL) return -1;
    
    if (save_string(file, course->name) != 0) return -1;
    if (fwrite(&course->coefficient, sizeof(double), 1, file) != 1) return -1;
    if (fwrite(&course->average, sizeof(double), 1, file) != 1) return -1;
    if (save_grades(file, &course->notes) != 0) return -1;
    
    return 0;
}


Course* load_course(FILE *file) {
    Course *course = (Course*)malloc(sizeof(Course));
    if (course == NULL) return NULL;
    
    course->name = load_string(file);
    if (course->name == NULL) {
        free(course);
        return NULL;
    }
    
    if (fread(&course->coefficient, sizeof(double), 1, file) != 1) {
        free(course->name);
        free(course);
        return NULL;
    }
    
    if (fread(&course->average, sizeof(double), 1, file) != 1) {
        free(course->name);
        free(course);
        return NULL;
    }
    
    if (load_grades(file, &course->notes) != 0) {
        free(course->name);
        free(course);
        return NULL;
    }
    
    return course;
}


int save_student(FILE *file, Student *student) {
    if (student == NULL) return -1;
    
    if (fwrite(&student->id, sizeof(int), 1, file) != 1) return -1;
    if (fwrite(&student->age, sizeof(int), 1, file) != 1) return -1;
    if (fwrite(&student->overall_average, sizeof(double), 1, file) != 1) return -1;
    if (save_string(file, student->first_name) != 0) return -1;
    if (save_string(file, student->last_name) != 0) return -1;
    if (fwrite(&student->courses_count, sizeof(int), 1, file) != 1) return -1;
    
    for (int i = 0; i < student->courses_count; i++) {
        if (save_course(file, student->courses[i]) != 0) return -1;
    }
    
    return 0;
}


Student* load_student(FILE *file) {
    Student *student = (Student*)malloc(sizeof(Student));
    if (student == NULL) return NULL;
    
    if (fread(&student->id, sizeof(int), 1, file) != 1) {
        free(student);
        return NULL;
    }
    
    if (fread(&student->age, sizeof(int), 1, file) != 1) {
        free(student);
        return NULL;
    }
    
    if (fread(&student->overall_average, sizeof(double), 1, file) != 1) {
        free(student);
        return NULL;
    }
    
    student->first_name = load_string(file);
    if (student->first_name == NULL) {
        free(student);
        return NULL;
    }
    
    student->last_name = load_string(file);
    if (student->last_name == NULL) {
        free(student->first_name);
        free(student);
        return NULL;
    }
    
    if (fread(&student->courses_count, sizeof(int), 1, file) != 1) {
        free(student->first_name);
        free(student->last_name);
        free(student);
        return NULL;
    }
    
    if (student->courses_count > 0) {
        student->courses = (Course**)malloc(student->courses_count * sizeof(Course*));
        if (student->courses == NULL) {
            free(student->first_name);
            free(student->last_name);
            free(student);
            return NULL;
        }
        
        for (int i = 0; i < student->courses_count; i++) {
            student->courses[i] = load_course(file);
            if (student->courses[i] == NULL) {
                for (int j = 0; j < i; j++) {
                    free_course(student->courses[j]);
                }
                free(student->courses);
                free(student->first_name);
                free(student->last_name);
                free(student);
                return NULL;
            }
        }
    } else {
        student->courses = NULL;
    }
    
    return student;
}


int save_class_to_binary(Class *class, const char *filepath) {
    if (class == NULL || filepath == NULL) {
        fprintf(stderr, "Error: Invalid parameters for save\n");
        return -1;
    }
    
    FILE *file = fopen(filepath, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot create file %s\n", filepath);
        return -1;
    }
    
    const char signature[] = "CLASSDATA";
    if (fwrite(signature, sizeof(char), strlen(signature), file) != strlen(signature)) {
        fprintf(stderr, "Error: Failed to write signature\n");
        fclose(file);
        return -1;
    }
    
    if (fwrite(&class->students_count, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to write students count\n");
        fclose(file);
        return -1;
    }
    
    for (int i = 0; i < class->students_count; i++) {
        if (save_student(file, class->students[i]) != 0) {
            fprintf(stderr, "Error: Failed to save student %d\n", i);
            fclose(file);
            return -1;
        }
    }
    
    if (fwrite(&class->courses_count, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to write courses count\n");
        fclose(file);
        return -1;
    }
    
    for (int i = 0; i < class->courses_count; i++) {
        if (save_course(file, class->courses[i]) != 0) {
            fprintf(stderr, "Error: Failed to save course %d\n", i);
            fclose(file);
            return -1;
        }
    }
    
    fclose(file);
    printf("Class data successfully saved to %s\n", filepath);
    return 0;
}


Class* load_class_from_binary(const char *filepath) {
    if (filepath == NULL) {
        fprintf(stderr, "Error: Invalid filepath\n");
        return NULL;
    }
    
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filepath);
        return NULL;
    }
    
    const char expected_signature[] = "CLASSDATA";
    char signature[10];
    if (fread(signature, sizeof(char), strlen(expected_signature), file) != strlen(expected_signature)) {
        fprintf(stderr, "Error: Failed to read signature\n");
        fclose(file);
        return NULL;
    }
    
    if (strncmp(signature, expected_signature, strlen(expected_signature)) != 0) {
        fprintf(stderr, "Error: Invalid file format\n");
        fclose(file);
        return NULL;
    }
    
    Class *class = (Class*)malloc(sizeof(Class));
    if (class == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    
    if (fread(&class->students_count, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read students count\n");
        free(class);
        fclose(file);
        return NULL;
    }
    
    if (class->students_count > 0) {
        class->students = (Student**)malloc(class->students_count * sizeof(Student*));
        if (class->students == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for students\n");
            free(class);
            fclose(file);
            return NULL;
        }
        
        for (int i = 0; i < class->students_count; i++) {
            class->students[i] = load_student(file);
            if (class->students[i] == NULL) {
                fprintf(stderr, "Error: Failed to load student %d\n", i);
                for (int j = 0; j < i; j++) {
                    free_student(class->students[j]);
                }
                free(class->students);
                free(class);
                fclose(file);
                return NULL;
            }
        }
    } else {
        class->students = NULL;
    }
    
    if (fread(&class->courses_count, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Error: Failed to read courses count\n");
        for (int i = 0; i < class->students_count; i++) {
            free_student(class->students[i]);
        }
        free(class->students);
        free(class);
        fclose(file);
        return NULL;
    }
    
    if (class->courses_count > 0) {
        class->courses = (Course**)malloc(class->courses_count * sizeof(Course*));
        if (class->courses == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for courses\n");
            for (int i = 0; i < class->students_count; i++) {
                free_student(class->students[i]);
            }
            free(class->students);
            free(class);
            fclose(file);
            return NULL;
        }
        
        for (int i = 0; i < class->courses_count; i++) {
            class->courses[i] = load_course(file);
            if (class->courses[i] == NULL) {
                fprintf(stderr, "Error: Failed to load course %d\n", i);
                for (int j = 0; j < i; j++) {
                    free_course(class->courses[j]);
                }
                free(class->courses);
                for (int j = 0; j < class->students_count; j++) {
                    free_student(class->students[j]);
                }
                free(class->students);
                free(class);
                fclose(file);
                return NULL;
            }
        }
    } else {
        class->courses = NULL;
    }
    
    fclose(file);
    printf("Class data successfully loaded from %s\n", filepath);
    printf("Loaded: %d students, %d courses\n", class->students_count, class->courses_count);
    
    return class;
}

int compare_students(const void *a, const void *b) {
    StudentRanking *sa = (StudentRanking*)a;
    StudentRanking *sb = (StudentRanking*)b;
    
    if (sb->average > sa->average) return 1;
    if (sb->average < sa->average) return -1;
    return 0;
}

Student** get_top_10(Class *class, int *count) {
    if (class == NULL || count == NULL) {
        return NULL;
    }
    
    *count = 0;
    
    // Compter les étudiants avec une moyenne valide
    int valid_students = 0;
    for (int i = 0; i < class->students_count; i++) {
        if (class->students[i]->courses_count > 0 && 
            class->students[i]->overall_average > 0) {
            valid_students++;
        }
    }
    
    if (valid_students == 0) {
        return NULL;
    }
    
    // Créer un tableau temporaire pour le tri
    StudentRanking *rankings = (StudentRanking*)malloc(valid_students * sizeof(StudentRanking));
    if (rankings == NULL) {
        return NULL;
    }
    
    // Remplir le tableau avec les étudiants ayant une moyenne
    int idx = 0;
    for (int i = 0; i < class->students_count; i++) {
        if (class->students[i]->courses_count > 0 && 
            class->students[i]->overall_average > 0) {
            rankings[idx].student = class->students[i];
            rankings[idx].average = class->students[i]->overall_average;
            idx++;
        }
    }
    
    // Trier par moyenne décroissante
    qsort(rankings, valid_students, sizeof(StudentRanking), compare_students);
    
    // Déterminer le nombre d'étudiants à retourner (max 10)
    int num_to_return = (valid_students < 10) ? valid_students : 10;
    
    // Allouer le tableau de résultats
    Student **top_students = (Student**)malloc(num_to_return * sizeof(Student*));
    if (top_students == NULL) {
        free(rankings);
        return NULL;
    }
    
    // Copier les meilleurs étudiants
    for (int i = 0; i < num_to_return; i++) {
        top_students[i] = rankings[i].student;
    }
    
    *count = num_to_return;
    free(rankings);
    
    return top_students;
}


Student** get_top_3_students_in_course(Class *class, const char *course_name, int *count) {
    if (class == NULL || course_name == NULL || count == NULL) {
        return NULL;
    }
    
    *count = 0;
    
    // Compter les étudiants inscrits dans cette matière avec des notes
    int enrolled_students = 0;
    for (int i = 0; i < class->students_count; i++) {
        Student *student = class->students[i];
        for (int j = 0; j < student->courses_count; j++) {
            if (strcmp(student->courses[j]->name, course_name) == 0 && 
                student->courses[j]->notes.size > 0) {
                enrolled_students++;
                break;
            }
        }
    }
    
    if (enrolled_students == 0) {
        return NULL;
    }
    
    // Créer un tableau temporaire pour le tri
    StudentRanking *rankings = (StudentRanking*)malloc(enrolled_students * sizeof(StudentRanking));
    if (rankings == NULL) {
        return NULL;
    }
    
    // Remplir le tableau avec les étudiants et leurs moyennes dans cette matière
    int idx = 0;
    for (int i = 0; i < class->students_count; i++) {
        Student *student = class->students[i];
        for (int j = 0; j < student->courses_count; j++) {
            if (strcmp(student->courses[j]->name, course_name) == 0 && 
                student->courses[j]->notes.size > 0) {
                rankings[idx].student = student;
                rankings[idx].average = student->courses[j]->average;
                idx++;
                break;
            }
        }
    }
    
    // Trier par moyenne décroissante
    qsort(rankings, enrolled_students, sizeof(StudentRanking), compare_students);
    
    // Allouer le tableau de résultats (toujours 3 places)
    Student **top_3 = (Student**)calloc(3, sizeof(Student*));
    if (top_3 == NULL) {
        free(rankings);
        return NULL;
    }
    
    // Copier les 3 meilleurs (ou moins s'il n'y en a pas assez)
    int num_to_copy = (enrolled_students < 3) ? enrolled_students : 3;
    for (int i = 0; i < num_to_copy; i++) {
        top_3[i] = rankings[i].student;
    }
    
    *count = num_to_copy;
    free(rankings);
    
    return top_3;
}




int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <data_file> [--save <binary_file>] [--load <binary_file>]\n", argv[0]);
        fprintf(stderr, "Examples:\n");
        fprintf(stderr, "  %s data.txt\n", argv[0]);
        fprintf(stderr, "  %s data.txt --save class.bin\n", argv[0]);
        fprintf(stderr, "  %s data.txt --load class.bin\n", argv[0]);
        return 1;
    }
    
    Class *class = NULL;
    
    // Vérifier si on doit charger depuis un fichier binaire
    if (argc >= 4 && strcmp(argv[2], "--load") == 0) {
        printf("Loading class from binary file: %s\n", argv[3]);
        class = load_class_from_binary(argv[3]);
        
        if (class == NULL) {
            fprintf(stderr, "Error: Failed to load class from binary file\n");
            return 1;
        }
        
        printf("\n=== Data loaded from binary file ===\n");
        display_class(class);
    }
    // Sinon, charger depuis le fichier texte
    else {
        printf("Loading class from text file: %s\n", argv[1]);
        class = load_class_from_file(argv[1]);
        
        if (class == NULL) {
            fprintf(stderr, "Error: Failed to load class data\n");
            return 1;
        }
        
        printf("\n=== Data loaded from text file ===\n");
        display_class(class);
        
        // Sauvegarder si demandé
        if (argc >= 4 && strcmp(argv[2], "--save") == 0) {
            printf("\nSaving class to binary file: %s\n", argv[3]);
            if (save_class_to_binary(class, argv[3]) != 0) {
                fprintf(stderr, "Error: Failed to save class to binary file\n");
                free_class(class);
                return 1;
            }
            printf("Save completed successfully!\n");
        }
    }
    
    // Libérer la mémoire
    free_class(class);
    
    return 0;
}
