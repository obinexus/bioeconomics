// odts_multivar.c – v2.4: Bioeconomic ODTS (C) – Production
// gcc -O2 -Wall -Wextra src/odts_multivar.c -o build/odts -lm && ./build/odts
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ITER  50
#define EPS       1e-12
#define LAMBDA    0.01
#define LAMBDA_UP 10.0
#define LAMBDA_DN 0.1

typedef struct {
    double x[2];
    double grad[2];
    double hessian[2][2];
    double residual;
    double lambda;
    int iter;
    int converged;
    char status[32];
} ODTS;

// ---------- f(x,y) ----------
double f(double x, double y) {
    return (x-1)*(x-1) + (y-2)*(y-2) + 0.1 * sin(10*x*y);
}

// ---------- Analytic Gradient ----------
void analytic_gradient(double x, double y, double g[2]) {
    double xy = 10 * x * y;
    double c = cos(xy);
    g[0] = 2*(x-1) + c;
    g[1] = 2*(y-2) + x * c;
}

// ---------- Analytic Hessian ----------
void analytic_hessian(double x, double y, double H[2][2]) {
    double xy = 10 * x * y;
    double s = sin(xy), c = cos(xy);
    H[0][0] = 2 - 10*s;
    H[0][1] = y * c;
    H[1][0] = H[0][1];
    H[1][1] = 2 - x * 10 * s;
}

// ---------- LM Step ----------
int lm_step(ODTS *sys) {
    double x = sys->x[0], y = sys->x[1];
    analytic_gradient(x, y, sys->grad);
    analytic_hessian(x, y, sys->hessian);

    // Damping
    sys->hessian[0][0] += sys->lambda;
    sys->hessian[1][1] += sys->lambda;

    double a = sys->hessian[0][0], b = sys->hessian[0][1];
    double c = sys->hessian[1][0], d = sys->hessian[1][1];
    double det = a*d - b*c;

    if (fabs(det) < EPS) {
        sys->lambda *= LAMBDA_UP;
        strcpy(sys->status, "DAMPING_UP");
        return 0;
    }

    double dx = (d * sys->grad[0] - b * sys->grad[1]) / det;
    double dy = (a * sys->grad[1] - c * sys->grad[0]) / det;

    sys->x[0] -= dx;
    sys->x[1] -= dy;

    double f_old = f(x, y);
    double f_new = f(sys->x[0], sys->x[1]);

    if (f_new < f_old - 1e-10) {
        sys->lambda *= LAMBDA_DN;
    } else {
        sys->lambda *= LAMBDA_UP;
    }
    return 1;
}

// ---------- CSV Log ----------
void log_csv(ODTS *sys, FILE *fp) {
    fprintf(fp, "%d,%.10f,%.10f,%.6e,%.6e,%.6e,%s,%.6f\n",
            sys->iter, sys->x[0], sys->x[1],
            sys->grad[0], sys->grad[1], sys->residual,
            sys->status, sys->lambda);
}

// ---------- JSON Audit ----------
void log_json(ODTS *sys) {
    FILE *fp = fopen("odts_audit_c.json", "w");
    time_t now = time(NULL);
    fprintf(fp, "{\n");
    fprintf(fp, "  \"model\": \"ODTS v2.4 Bioeconomic C\",\n");
    fprintf(fp, "  \"timestamp\": \"%s\",\n", ctime(&now));
    fprintf(fp, "  \"status\": \"%s\",\n", sys->status);
    fprintf(fp, "  \"optimal\": [%.10f, %.10f],\n", sys->x[0], sys->x[1]);
    fprintf(fp, "  \"f_opt\": %.6e\n", f(sys->x[0], sys->x[1]));
    fprintf(fp, "  \"iterations\": %d\n", sys->iter);
    fprintf(fp, "}\n");
    fclose(fp);
}

// ---------- Engine ----------
int run_odts(ODTS *sys) {
    sys->iter = 0;
    sys->lambda = LAMBDA;
    sys->converged = 0;
    strcpy(sys->status, "RUNNING");

    FILE *csv = fopen("odts_trace_c.csv", "w");
    fprintf(csv, "iter,x,y,gx,gy,res,status,lambda\n");
    log_csv(sys, csv);

    for (sys->iter = 1; sys->iter <= MAX_ITER; sys->iter++) {
        lm_step(sys);

        sys->residual = sqrt(sys->grad[0]*sys->grad[0] + sys->grad[1]*sys->grad[1]);

        if (sys->residual < EPS) {
            strcpy(sys->status, "CONVERGED");
            sys->converged = 1;
            break;
        }

        log_csv(sys, csv);

        if (sys->lambda > 1e8) {
            strcpy(sys->status, "OVERDAMPED");
            break;
        }
    }

    log_csv(sys, csv);
    fclose(csv);
    log_json(sys);

    return sys->converged;
}

// ---------- Main ----------
int main() {
    ODTS sys = {0};
    sys.x[0] = 0.3;
    sys.x[1] = 0.8;

    printf("=== ODTS v2.4 – Bioeconomic Engine (C) ===\n");
    printf("Start: (%.3f, %.3f)\n\n", sys.x[0], sys.x[1]);

    if (run_odts(&sys)) {
        printf("CONVERGED @ iter %d → x* = (%.10f, %.10f)\n", sys.iter, sys.x[0], sys.x[1]);
    } else {
        printf("DIVERGED: %s (lambda=%.2e)\n", sys.status, sys.lambda);
    }

    printf("Final f* = %.6e\n", f(sys.x[0], sys.x[1]));
    printf("Audit: odts_trace_c.csv + odts_audit_c.json\n");
    return 0;
}
