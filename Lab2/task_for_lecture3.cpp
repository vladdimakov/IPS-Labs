#include <chrono>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
#include <ctime>
#include <stdio.h>

using namespace std::chrono;

// ���������� ����� � �������� ���������� �������
const int MATRIX_SIZE = 1500;

/// ������� InitMatrix() ��������� ���������� � ��������
/// ��������� ���������� ������� ���������� ����������
/// matrix - �������� ������� ����
void InitMatrix(double** matrix)
{
    for (int i = 0; i < MATRIX_SIZE; ++i)
    {
        matrix[i] = new double[MATRIX_SIZE + 1];
    }

    for (int i = 0; i < MATRIX_SIZE; ++i)
    {
        for (int j = 0; j <= MATRIX_SIZE; ++j)
        {
            matrix[i][j] = rand() % 2500 + 1;
        }
    }
}

/// ������� InitTestMatrix() ��������� ���������� � ��������
/// ��������� ���������� ������� ����������, ����������� � �������
/// matrix - �������� ������� ����
void InitTestMatrix(double** test_matrix)
{
    // ���-�� ����� � �������, ���������� � �������� �������
    const int test_matrix_lines = 4;

    // ���� �� �������
    for (int i = 0; i < test_matrix_lines; ++i)
    {
        // (test_matrix_lines + 1)- ���������� �������� � �������� �������,
        // ��������� ������� ������� ������� ��� ������ ����� ���������, �������� � ����
        test_matrix[i] = new double[test_matrix_lines + 1];
    }

    // ������������� �������� �������
    test_matrix[0][0] = 2;
    test_matrix[0][1] = 5;
    test_matrix[0][2] = 4;
    test_matrix[0][3] = 1;
    test_matrix[0][4] = 20;
    test_matrix[1][0] = 1;
    test_matrix[1][1] = 3;
    test_matrix[1][2] = 2;
    test_matrix[1][3] = 1;
    test_matrix[1][4] = 11;
    test_matrix[2][0] = 2;
    test_matrix[2][1] = 10;
    test_matrix[2][2] = 9;
    test_matrix[2][3] = 7;
    test_matrix[2][4] = 40;
    test_matrix[3][0] = 3;
    test_matrix[3][1] = 8;
    test_matrix[3][2] = 9;
    test_matrix[3][3] = 2;
    test_matrix[3][4] = 37;
}

/// ������� SerialGaussMethod() ������ ���� ������� ������ ���������������
/// matrix - �������� ������� �������������� ���������, �������� � ����,
/// ��������� ������� ������� - �������� ������ ������ ���������
/// rows - ���������� ����� � �������� �������
/// result - ������ ������� ����
void SerialGaussMethod(double** matrix, const int rows, double* result)
{
    int k;
    double koef;

    high_resolution_clock::time_point t1, t2;
    t1 = high_resolution_clock::now();

    // ������ ��� ������ ������
    for (k = 0; k < rows; ++k)
    {
        //
        for (int i = k + 1; i < rows; ++i)
        {
            koef = -matrix[i][k] / matrix[k][k];

            for (int j = k; j <= rows; ++j)
            {
                matrix[i][j] += koef * matrix[k][j];
            }
        }
    }

    t2 = high_resolution_clock::now();
    duration<double> duration = (t2 - t1);
    printf("Serial Gauss method direct pass duration: %g (seconds)\n\n", duration.count());

    // �������� ��� ������ ������
    result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

    for (k = rows - 2; k >= 0; --k)
    {
        result[k] = matrix[k][rows];

        //
        for (int j = k + 1; j < rows; ++j)
        {
            result[k] -= matrix[k][j] * result[j];
        }

        result[k] /= matrix[k][k];
    }
}

/// ������� ParallelGaussMethod() ������ ���� ������� ������ �����������
/// matrix - �������� ������� �������������� ���������, �������� � ����,
/// ��������� ������� ������� - �������� ������ ������ ���������
/// rows - ���������� ����� � �������� �������
/// result - ������ ������� ����
void ParallelGaussMethod(double** matrix, const int rows, double* result)
{
    int k;

    high_resolution_clock::time_point t1, t2;
    t1 = high_resolution_clock::now();

    // ������ ��� ������ ������
    for (k = 0; k < rows; ++k)
    {
        cilk_for(int i = k + 1; i < rows; ++i)
        {
            double koef = -matrix[i][k] / matrix[k][k];

            for (int j = k; j <= rows; ++j)
            {
                matrix[i][j] += koef * matrix[k][j];
            }
        }
    }

    t2 = high_resolution_clock::now();
    duration<double> duration = (t2 - t1);
    printf("Parallel Gauss method direct pass duration: %g (seconds)\n\n", duration.count());

    // �������� ��� ������ ������
    result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

    for (k = rows - 2; k >= 0; --k)
    {
        cilk::reducer_opadd<double> resultTmp(matrix[k][rows]);

        cilk_for(int j = k + 1; j < rows; ++j)
        {
            resultTmp -= matrix[k][j] * result[j];
        }

        result[k] = resultTmp.get_value() / matrix[k][k];
    }
}

int main()
{
    srand((unsigned)time(0));

    bool useTestMatrix = false;

    const int matrix_lines = useTestMatrix ? 4 : MATRIX_SIZE;
    double** matrix = new double*[matrix_lines];
    useTestMatrix ? InitTestMatrix(matrix) : InitMatrix(matrix);

    // ������ ������� ����
    double* result = new double[matrix_lines];

    // SerialGaussMethod(matrix, matrix_lines, result);
    ParallelGaussMethod(matrix, matrix_lines, result);

    for (int i = 0; i < matrix_lines; ++i)
    {
        delete[] matrix[i];
    }

    delete[] matrix;

    if (useTestMatrix)
    {
        printf("Solution:\n");
        for (int i = 0; i < matrix_lines; ++i)
        {
            printf("x(%d) = %lf\n", i, result[i]);
        }
    }

    delete[] result;

    return 0;
}
