# Two-page handwritten appendix guide — Task F

Copy the derivation below by hand, show intermediate chain-rule steps, sign/date it if required, and scan/photograph the pages. Do not submit this typed guide as a handwritten scan.

Given: `x=1`, `y=2`, `h=tanh(wx+b)`, `y_hat=vh+c`, `J=1/2(y_hat-y)^2`; initial `w=0.5,b=0,v=1,c=0`, learning rate `eta=0.1`.

Forward pass:
- `z=wx+b=0.5`
- `h=tanh(0.5)=0.462117157260`
- `y_hat=1(0.462117157260)+0=0.462117157260`
- `e=y_hat-y=-1.537882842740`
- `J=0.5 e^2=1.182541819`

Chain rule:
- `dJ/dy_hat=e`
- `dy_hat/dv=h`, `dy_hat/dc=1`, `dy_hat/dh=v`
- `dh/dz=1-h^2`, `dz/dw=x`, `dz/db=1`
- `dJ/dv=e h=-0.710682047486`
- `dJ/dc=e=-1.537882842740`
- `dJ/dw=e v (1-h^2)x=-1.209464475240`
- `dJ/db=e v (1-h^2)=-1.209464475240`

Simultaneous update (all gradients evaluated at original parameters):
- `w_new=w-eta*dw=0.620946447524`
- `b_new=b-eta*db=0.120946447524`
- `v_new=v-eta*dv=1.071068204748`
- `c_new=c-eta*dc=0.153788284274`

Gradient direction: all four gradients are negative, so subtracting `eta*gradient` increases all four parameters. This moves the prediction toward the target for this sample at the initial point. Include the central finite-difference check from the CLI output and explain that it approximates each derivative using symmetric perturbations `epsilon=1e-6`.
