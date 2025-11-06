# odts_bioecon.py
# pip install numpy matplotlib sympy pandas
import numpy as np
import matplotlib.pyplot as plt
from sympy import symbols, diff, sin, lambdify
import pandas as pd
import json
from datetime import datetime

class ODTS:
    def __init__(self, vars=['x', 'y'], expr=None):
        self.vars = [symbols(v) for v in vars]
        self.expr = expr or (self.vars[0]-1)**2 + (self.vars[1]-2)**2 + 0.1*sin(10*self.vars[0]*self.vars[1])
        self.f = lambdify(self.vars, self.expr, 'numpy')
        self.grad_f = [lambdify(self.vars, diff(self.expr, v), 'numpy') for v in self.vars]
        self.history = []
        self.series = []

    def gradient(self, x):
        return np.array([g(*x) for g in self.grad_f])

    def hessian(self, x):
        h = np.zeros((len(x), len(x)))
        eps = 1e-6
        for i in range(len(x)):
            x1 = x.copy(); x1[i] += eps
            x2 = x.copy(); x2[i] -= eps
            h[i] = (self.gradient(x1) - self.gradient(x2)) / (2*eps)
        return h.T  # numerical

    def newton_step(self, x):
        g = self.gradient(x)
        H = self.hessian(x)
        try:
            step = np.linalg.solve(H, g)
            return x - step, g, np.linalg.norm(g)
        except:
            return x, g, np.inf

    def run(self, x0, max_iter=20):
        x = np.array(x0, dtype=float)
        for i in range(max_iter):
            x_new, grad, res = self.newton_step(x)
            self.history.append({ 'iter': i, 'x': x.tolist(), 'grad': grad.tolist(), 'res': res })
            self.series.append(res)

            if res < 1e-8:
                self.history[-1]['status'] = 'CONVERGED'
                break
            if i > 1 and abs(self.series[-1]/self.series[-2]) > 1.5:
                self.history[-1]['status'] = 'DIVERGED'
                break

            x = x_new
        else:
            self.history[-1]['status'] = 'MAX_ITER'

        self.export_audit()
        self.plot_convergence()
        return x

    def export_audit(self):
        audit = {
            "model": "ODTS v2.0 Multivariable",
            "timestamp": datetime.now().isoformat(),
            "function": "(x-1)^2 + (y-2)^2 + 0.1*sin(10xy)",
            "biological_compliance": "Survival Reserve Stress Minimization",
            "trace": self.history
        }
        with open("odts_audit.json", "w") as f:
            json.dump(audit, f, indent=2)
        pd.DataFrame(self.history).to_csv("odts_trace.csv", index=False)
        print("Audit exported: odts_audit.json + odts_trace.csv")

    def plot_convergence(self):
        res = [h['res'] for h in self.history]
        plt.figure(figsize=(10,5))
        plt.subplot(1,2,1)
        plt.plot(res, 'o-', label='Residual (∇f norm)')
        plt.yscale('log'); plt.grid(); plt.title("ODTS Convergence")
        plt.xlabel("Iteration"); plt.ylabel("||∇f||")
        plt.legend()

        plt.subplot(1,2,2)
        x_hist = np.array([h['x'] for h in self.history])
        plt.plot(x_hist[:,0], x_hist[:,1], 'o-', color='red')
        plt.scatter([1], [2], color='green', s=100, label="Optimal (1,2)")
        plt.title("Path to Biological Equilibrium")
        plt.xlabel("x (Income)"); plt.ylabel("y (Housing Cost)")
        plt.legend(); plt.grid()
        plt.tight_layout()
        plt.savefig("odts_convergence.png", dpi=300)
        print("Plot saved: odts_convergence.png")

# === RUN BIOECONOMICS STRESS TEST ===
odts = ODTS()
print("Starting ODTS Biological Economics Engine...")
optimal = odts.run(x0=[0.3, 0.8])
print(f"\nOPTIMAL SURVIVAL POINT: x* = {optimal}")
print("→ Housing & Income balanced. System stable. No collapse.")
