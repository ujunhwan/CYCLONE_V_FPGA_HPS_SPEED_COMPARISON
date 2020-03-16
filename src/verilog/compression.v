module compression(

input [31:0] data_in,
output reg [31:0] data_out,
input clk,
input rst,
output ready_in,
input valid_in,
input ready_out,
output valid_out
);

assign ready_in = ready_out;
assign valid_out = valid_in;

always@(clk) begin
casex (data_in)
32'b0000000000000001xxxxxxxxxxxxxxxx: data_out = {4'b0000, data_in[15:4]};
32'b000000000000001xxxxxxxxxxxxxxxxx: data_out = {4'b0001, data_in[16:5]};
32'b00000000000001xxxxxxxxxxxxxxxxxx: data_out = {4'b0010, data_in[17:6]};
32'b0000000000001xxxxxxxxxxxxxxxxxxx: data_out = {4'b0011, data_in[18:7]};
32'b000000000001xxxxxxxxxxxxxxxxxxxx: data_out = {4'b0100, data_in[19:8]};
32'b00000000001xxxxxxxxxxxxxxxxxxxxx: data_out = {4'b0101, data_in[20:9]};
32'b0000000001xxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b0110, data_in[21:10]};
32'b000000001xxxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b0111, data_in[22:11]};
32'b00000001xxxxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b1000, data_in[23:12]};
32'b0000001xxxxxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b1001, data_in[24:13]};
32'b000001xxxxxxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b1010, data_in[25:14]};
32'b00001xxxxxxxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b1011, data_in[26:15]};
32'b0001xxxxxxxxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b1100, data_in[27:16]};
32'b001xxxxxxxxxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b1101, data_in[28:17]};
32'b01xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b1110, data_in[29:18]};
32'b1xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx: data_out = {4'b1111, data_in[30:19]};
default : data_out = {4'b0000, data_in[14:3]};
endcase
end

endmodule
