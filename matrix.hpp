#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstring>

template<typename T>
T** raw_matrix_alloc(const size_t rows, const size_t cols){
    T** matrix = (int**)malloc(rows * sizeof(T *));
    for(int i = 0; i < rows; ++i){
        matrix[i] = (T*)malloc(cols * sizeof(T));
        memset(matrix[i], 0, cols * sizeof(T));
    }
    return matrix;
}

template <typename T>
class Matrix{
    public:
        uint32_t cols;
        uint32_t rows;
        T** data;

        Matrix(uint32_t r, uint32_t c){
            rows = r;
            cols = c;
            data = raw_matrix_alloc<T>(rows, cols);
        };

        Matrix(uint32_t r, uint32_t c, T** d){
            rows = r;
            cols = c;
            data = d;
        };

        ~Matrix(){
            if(data == nullptr){
                return;
            }
            for(size_t i = 0; i < rows; ++i){
                delete[] data[i];
            }
            delete[] data;
        }

        Matrix clone(){
            Matrix<T> clone(rows, cols);
            for(size_t i = 0; i < rows; ++i){
                for(size_t j = 0; j < cols; ++j){
                    clone.data[i][j] = data[i][j];
                }
            }
            return clone;
        }

        Matrix clone_scale(int scale){
            Matrix<T> clone(rows, cols);
            for(size_t i = 0; i < rows; ++i){
                for(size_t j = 0; j < cols; ++j){
                    clone.data[i][j] = data[i][j] * scale;
                }
            }
            return clone;
        }

        Matrix<T>* operator*(Matrix<T>* const __restrict__ b){
            if (cols == b->rows){
                Matrix<T>* c = matrix_alloc( rows, b->cols);
                for(int i = 0; i < rows; ++i){
                    for(int j = 0; j < cols; ++j){
                        for(int k = 0; k < b->cols; ++k){
                            c->data[i][k] += data[i][j] * b->data[j][k];
                        }
                    }
                }
                return c;
            }
            else {
                std::cout << "matrices not compatible\n";
                return NULL;
            }
        }

        Matrix<T>(Matrix<T>&t){
            cols = t.cols;
            rows = t.rows;
            data = t.data;
            t.data = nullptr;
        }

        Matrix<T> operator*(Matrix b){
            std::cout << "not reccomended use matrix_mul fn 1\n";
            if (cols == b.rows){
                T** result = raw_matrix_mul(data, b.data, rows, cols, b.rows, b.cols);
                //Matrix<T> temp(rows, b.cols, result);
                return Matrix<T>(rows, b.cols, result);
            }
            else {
                std::cout << "matrices not compatible\n";
                return Matrix<T>(1,1);
            }
        }

        Matrix operator*(int scalar){
            return this->clone_scale(scalar);
        }

        void operator*=(int scalar){
            for(size_t i = 0; i < rows; ++i){
                for(size_t j = 0; j < rows; ++j){
                    data[i][j] *= scalar;
                }
            }
        }

        void print_matrix(){
            for(size_t i = 0; i < rows; ++i){
                for(size_t j = 0; j < cols; ++j){
                    std::cout << data[i][j] << "\t";
                }
                std::cout << "\n";
            }
        }
    private:
        Matrix<T>* matrix_alloc(uint32_t rows, uint32_t cols){
            Matrix<T>* matrix = new Matrix<T>(rows, cols);
            matrix->data = raw_matrix_alloc<T>(rows, cols);
            return matrix;
        }

        T** raw_matrix_mul(T ** __restrict__ a, T **__restrict__ b,
                const int a_rows, const size_t a_cols, const size_t b_rows, const int b_cols) {
            if (a_cols == b_rows){
                T** c = raw_matrix_alloc<T>(a_rows, b_cols);
                for(int i = 0; i < a_rows; ++i){
                    for(int j = 0; j < a_cols; ++j){
                        for(int k = 0; k < b_cols; ++k){
                            c[i][k] += a[i][j] * b[j][k];
                        }
                    }
                }
                return c;
            }
            else {
                printf("matrices not compatible\n");
                return NULL;
            }
        }
};

template<typename T>
Matrix<T>* matrix_alloc(uint32_t rows, uint32_t cols){
    Matrix<T>* matrix = new Matrix<T>(rows, cols);
    matrix->data = raw_matrix_alloc<T>(rows, cols);
    return matrix;
}


template <typename T>
Matrix<T>* matrix_mul(Matrix<T>* const __restrict__ a, Matrix<T>* const __restrict__ b){
    if (a->cols == b->rows){
        Matrix<T>* c = matrix_alloc<T>(a->rows, b->cols);
        for(int i = 0; i < a->rows; ++i){
            for(int j = 0; j < a->cols; ++j){
                for(int k = 0; k < b->cols; ++k){
                    c->data[i][k] += a->data[i][j] * b->data[j][k];
                }
            }
        }
        return c;
    }
    else {
        std::cout << "matrices not compatible\n";
        return NULL;
    }
}

template <typename T>
T** raw_matrix_dealloc(T** matrix, size_t rows, size_t cols){
    for(size_t i = 0; i < rows; ++i){
        free(matrix[i]);
    }
    return matrix;
}

template<typename T>
T** raw_matrix_mul(const T **const __restrict__ a, const int **const __restrict__ b,
        const int a_rows, const size_t a_cols, const size_t b_rows, const int b_cols) {
    if (a_cols == b_rows){
        T** c = raw_matrix_alloc<T>(a_rows, b_cols);
        for(int i = 0; i < a_rows; ++i){
            for(int j = 0; j < a_cols; ++j){
                for(int k = 0; k < b_cols; ++k){
                    c[i][k] += a[i][j] * b[j][k];
                }
            }
        }
        return c;
    }
    else {
        printf("matrices not compatible\n");
        return NULL;
    }
}

#endif
