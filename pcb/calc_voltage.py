#%%

# offset = 0.5
# amp = 200
# offset_v = 3.0 * 100/(270 + 100)
# offset_current = 3/(270 + 100)
# offset_v = 3.0 * 100/(680 + 100)
# offset_current = 3/(680 + 100)
offset_v = 3.0 * 100/(1000 + 100)
offset_current = 3/(1000 + 100)

print("offset_v =", offset_v)
print("offset_current:", offset_current)
print("offset_watt", 3*offset_current)
amp = 100000/(100 + 270)
# amp = 100000/(100 + 330)
print("amp =", amp)
emf_neg50deg = -1.889/1000.
emf_neg20deg = -0.778/1000.
emf_0deg = 0.
emf_100deg = 4.096 / 1000.0
emf_250deg = 10.153/1000.
emf_270deg = 10.971/1000.
emf_280deg = 11.382/1000.
emf_290deg = 11.795/1000.
emf_300deg = 12.209/1000.

outneg50 = offset_v + emf_neg50deg * amp
outneg20 = offset_v + emf_neg20deg * amp
out0 = offset_v + emf_0deg * amp
out100 = offset_v + emf_100deg * amp
out250 = offset_v + emf_250deg * amp
out270 = offset_v + emf_270deg * amp
out280 = offset_v + emf_280deg * amp
out290 = offset_v + emf_290deg * amp
out300 = offset_v + emf_300deg * amp

print("outneg50", outneg50)
print("outneg20", outneg20)
print("out100", out100)
print("out250", out250)
print("out270", out270)
print("out280", out280)
print("out290", out290)
print("out300", out300)
