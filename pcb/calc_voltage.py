#%%

# offset = 0.5
# amp = 200
# offset_v = 3.0 * 100/(270 + 100)
# offset_current = 3/(270 + 100)
# offset_v = 3.0 * 100/(680 + 100)
# offset_current = 3/(680 + 100)

r4 = 10
r5 = 1.5

vref = 1.8
offset_v = vref * r5 / (r4 + r5)
offset_current = vref / (r4 + r5)

print("offset_v =", offset_v)
print("offset_current:", offset_current)
print("offset_watt", vref * offset_current)
gain = 560 / (1.5 + 1.5)
# amp = 100000 / (100 + 390)
print("amp =", gain)
emf_neg50deg = -1.889/1000.
emf_neg20deg = -0.778/1000.
emf_0deg = 0.
emf_100deg = 4.096 / 1000.0
emf_200deg = 8.138 / 1000.0
emf_250deg = 10.153/1000.
emf_270deg = 10.971/1000.
emf_280deg = 11.382/1000.
emf_290deg = 11.795/1000.
emf_300deg = 12.209/1000.

outneg50 = offset_v + emf_neg50deg * gain
outneg20 = offset_v + emf_neg20deg * gain
out0 = offset_v + emf_0deg * gain
out100 = offset_v + emf_100deg * gain
out200 = offset_v + emf_200deg * gain
out250 = offset_v + emf_250deg * gain
out270 = offset_v + emf_270deg * gain
out280 = offset_v + emf_280deg * gain
out290 = offset_v + emf_290deg * gain
out300 = offset_v + emf_300deg * gain

print("outneg50", outneg50)
print("outneg20", outneg20)
print("out100", out100)
print("out200", out200)
print("out250", out250)
print("out270", out270)
print("out280", out280)
print("out290", out290)
print("out300", out300)

# %%
