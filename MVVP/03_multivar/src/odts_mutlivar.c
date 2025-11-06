// odts_multivar.c
// gcc odts_multivar.c -o odts -lm && ./odts
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define MAX_VARS 5
#define MAX_ORDER 4
#define EPS 1e-8

typedef struct {
    double x[MAX_VARS];
    double grad[MAX_VARS];
    double hessian[MAX_VARS][MAX_VARS];
    double series[MAX_ORDER + 1];
    int order;
    int converged;
    double residual;
} ODTS;

void print_vec(const char* name, double* v, int n) {
    printf("%s: [", name);
    for (int i = 0; i < n; i++) printf("%.6f%s", v[i], i<n-1?", ":"");
    printf("]\n");
}

void print_matrix(const char* name, double m[MAX_VARS][MAX_VARS], int n) {
    printf("%s:\n", name);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) printf(" %.6f", m[i][j]);
        printf("\n");
    }
}

// Example: f(x,y) = (x-1)^2 + (y-2)^2 + 0.1*sin(10*x*y) → biological stress function
double f(double* x) {
    return pow(x[0]-1, 2) + pow(x[1]-2, 2) + 0.1 * sin(10 * x[0] * x[1]);
}

void compute_gradient(double* x, double* grad, int n) {
    double h = 1e-6;
    double f0 = f(x);
    for (int i = 0; i < n; i++) {
        double x_save = x[i];
        x[i] += h;
        grad[i] = (f(x) - f0) / h;
        x[i] = x_save;
    }
}

void compute_hessian(double* x, double hsn[MAX_VARS][MAX_VARS], int n) {
    double grad_p[MAX_VARS], grad_m[MAX_VARS], grad0[MAX_VARS];
    double h = 1e-5;
    compute_gradient(x, grad0, n);
    for (int i = 0; i < n; i++) {
        double x_save = x[i];
        x[i] += h; compute_gradient(x, grad_p, n);
        x[i] -= 2*h; compute_gradient(x, grad_m, n);
        x[i] = x_save;
        for (int j = 0; j < n; j++) {
            hsn[i][j] = (grad_p[j] - 2*grad0[j] + grad_m[j]) / (h*h);
        }
    }
}

int check_convergence(ODTS* sys, int n) {
    double norm = 0;
    for (int i = 0; i < n; i++) norm += sys->grad[i] * sys->grad[i];
    sys->residual = sqrt(norm);

    // Series: check if higher-order terms → 0
    sys->series[sys->order] = sys->residual;
    if (sys->order > 1) {
        double ratio = fabs(sys->series[sys->order] / sys->series[sys->order-1]);
        sys->converged = (ratio < 0.9 && sys->residual < 1e-4);
        return sys->converged;
    }
    return 0;
}

int main() {
    ODTS sys = {0};
    sys.x[0] = 0.5; sys.x[1] = 1.0;  // starting guess: survival reserve stress point
    sys.order = 0;

    printf("=== ODTS v2.0 – Multivariable Convergence Engine ===\n");
    printf("Function: f(x,y) = (x-1)^2 + (y-2)^2 + 0.1*sin(10xy)\n");
    printf("Goal: Minimize stress → converge to (1,2)\n\n");

    for (int iter = 0; iter < 20; iter++) {
        compute_gradient(sys.x, sys.grad, 2);
        compute_hessian(sys.x, sys.hessian, 2);
        sys.order++;

        if (check_convergence(&sys, 2)) {
            printf("CONVERGED at iteration %d\n", iter);
            break;
        }

        // Newton step: x = x - H⁻¹∇f
        double det = sys.hessian[0][0]*sys.hessian[1][1] - sys.hessian[0][1]*sys.hessian[1][0];
        if (fabs(det) < EPS) { printf("DIVERGED: Hessian singular\n"); return 1; }

        double dx = -(sys.hessian[1][1]*sys.grad[0] - sys.hessian[0][1]*sys.grad[1]) / det;
        double dy = -(sys.hessian[0][0]*sys.grad[1] - sys.hessian[1][0]*sys.grad[0]) / det;

        sys.x[0] += dx; sys.x[1] += dy;

        printf("Iter %2d | x=(%.6f, %.6f) | ∇f=", iter, sys.x[0], sys.x[1]);
        print_vec("", sys.grad, 2);
        printf("           | residual=%.2e | order=%d\n", sys.residual, sys.order);
    }

    printf("\nFINAL STATE:\n");
    print_vec("x*", sys.x, 2);
    print_vec("∇f", sys.grad, 2);
    printf("f(x*) = %.6f\n", f(sys.x));
    printf("Series residual: %.2e → %s\n", sys.residual, sys.converged ? "CONVERGED" : "DIVERGED");
    printf("================================================\n");

    return 0;
}
