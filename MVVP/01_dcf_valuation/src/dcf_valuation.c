// dcf_valuation.c
// Compile: gcc dcf_valuation.c -o dcf -lm
#include <stdio.h>
#include <math.h>

typedef struct {
    double fcf[5];      // 5-year explicit forecast
    double growth;      // terminal growth
    double wacc;        // discount rate
    double net_debt;
} DCFInput;

double terminal_value(double last_fcf, double g, double r) {
    return last_fcf * (1 + g) / (r - g);
}

double enterprise_value(DCFInput *in) {
    double pv = 0.0;
    for (int t = 0; t < 5; t++) {
        pv += in->fcf[t] / pow(1 + in->wacc, t + 1);
    }
    double tv = terminal_value(in->fcf[4], in->growth, in->wacc);
    pv += tv / pow(1 + in->wacc, 5);
    return pv;
}

int main() {
    DCFInput target = {
        .fcf = {120.5, 138.0, 158.7, 180.2, 200.0},  // £m
        .growth = 0.025,
        .wacc = 0.085,
        .net_debt = 450.0
    };

    double ev = enterprise_value(&target);
    double equity = ev - target.net_debt;

    printf("=== M&A DCF VALUATION (London Target) ===\n");
    printf("Enterprise Value: £%.1f m\n", ev);
    printf("Net Debt: £%.1f m\n", target.net_debt);
    printf("Equity Value: £%.1f m\n", equity);
    printf("Implied Share Price (@ 85m shares): £%.2f\n", equity / 85.0);
    printf("========================================\n");

    return 0;
}
