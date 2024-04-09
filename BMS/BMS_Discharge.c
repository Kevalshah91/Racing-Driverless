#include <stdio.h>
#define ROWS 16
#define COLS 8

void main() {
    int i,j;
    int flag=0;
    // int discharge_bms[ROWS][COLS];
    // 1 = Discharge
    //threshold = 0.05
    //max = 4.2 
    //min = 3.1 
    float array[ROWS][COLS] = {
        {3.5, 4.0, 3.8, 4.2, 3.9, 3.7, 3.2, 3.6},
        {3.6, 3.9, 3.4, 3.3, 4.1, 3.5, 3.1, 4.0},
        {3.2, 3.7, 3.1, 3.5, 3.9, 3.3, 4.2, 3.8},
        {4.0, 3.3, 3.8, 3.6, 3.4, 3.7, 4.1, 3.2},
        {3.8, 4.1, 3.6, 3.9, 3.2, 3.4, 3.5, 3.1},
        {3.5, 3.9, 4.0, 3.3, 3.8, 3.2, 3.6, 4.2},
        {3.1, 3.7, 3.2, 3.6, 3.9, 3.5, 3.4, 4.0},
        {4.1, 3.8, 3.5, 3.4, 3.3, 3.6, 4.2, 3.2},
        {3.9, 4.0, 3.6, 3.7, 3.2, 3.5, 3.8, 3.3},
        {3.3, 3.6, 3.9, 3.8, 3.4, 4.1, 3.7, 3.2},
        {4.0, 3.2, 3.5, 3.9, 3.6, 3.1, 4.2, 3.4},
        {3.8, 3.4, 4.1, 3.3, 3.7, 3.5, 3.2, 4.0},
        {4.2, 3.1, 3.6, 3.9, 3.4, 3.8, 3.5, 3.7},
        {3.3, 3.8, 3.5, 4.0, 3.6, 3.9, 3.7, 3.2},
        {3.6, 3.9, 3.2, 3.7, 3.4, 4.1, 3.8, 3.5},
        {4.0, 3.5, 3.3, 3.8, 3.1, 3.6, 4.2, 3.9}
    };
    printf("Array:\n");
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            printf("%.1f ", array[i][j]);
        }
        printf("\n");
    }
    float min = array[0][0];
    int minr;
    int minc;
    for(i = 0; i < ROWS; i++) {
        for(j = 0; j < COLS; j++) {
            if(array[i][j] < min) {
                min = array[i][j];
                minr = i;
                minc = j;
            }
        }
    }
    printf("Min value: %.1f\n", min);
    printf("At position: %d, %d\n", minr + 1, minc + 1);
    float threshold = 0.05; 
    int discharge_bms[ROWS][COLS];
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (array[i][j] - min >= threshold) {
                discharge_bms[i][j] = 1;
            } else {
                discharge_bms[i][j] = 0;
            }
        }
    }
    printf("Flag values : \n");
    for(i=0;i<ROWS;i++){
        for(j=0;j<COLS;j++){
            printf("%d ",discharge_bms[i][j]);
        }
        printf("\n");
    }
    printf("Odd split : \n");
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if ((i + j) % 2 == 0) {
                printf("%d ", discharge_bms[i][j]);
            } else {
                printf("%d ", flag); 
            }
        }
        printf("\n");
    }
    printf("Even splt : \n");
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if ((i + j) % 2 != 0) {
                printf("%d ", discharge_bms[i][j]);
            } else {
                printf("%d ", flag); 
            }
        }
        printf("\n");
    }
}
