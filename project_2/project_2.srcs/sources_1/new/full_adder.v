//==================================================
// FULL ADDER USING HALF ADDERS
//==================================================
module full_adder (
    input  a,
    input  b,
    input  cin,
    output sum,
    output cout
);

    wire s1;   // Sum from first half adder
    wire c1;   // Carry from first half adder
    wire c2;   // Carry from second half adder

    // First Half Adder
    half_adder HA1 (
        .a(a),
        .b(b),
        .sum(s1),
        .carry(c1)
    );

    // Second Half Adder
    half_adder HA2 (
        .a(s1),
        .b(cin),
        .sum(sum),
        .carry(c2)
    );

    // Final carry
    assign cout = c1 | c2;

endmodule



