import snappy
from spherogram import DTcodec

def same_knot_or_mirror(dt1, dt2):
    K1 = DTcodec(dt1).link()
    K2 = DTcodec(dt2).link()

    M1 = K1.exterior()
    M2 = K2.exterior()
    M2m = K2.mirror().exterior()

    return (
        M1.is_isometric_to(M2) or
        M1.is_isometric_to(M2m)
    )

dt1 = [4, 8, 10, 12, 2, 14, 6]
dt2 = [4, 8, 12, 10, 2, 14, 6]

print(same_knot_or_mirror(dt1, dt2))