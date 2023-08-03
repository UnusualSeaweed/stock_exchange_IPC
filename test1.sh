make
gcc ./pe_exchange.c ./orderbook.c ./pipe_related.c ./my_signals.c -lm -o ./pe_exchange
gcc ./tests/dud_trader.c -o ./tests/dud_trader
gcc ./tests/one_order.c -o ./tests/one_order
# executing file 
# ./pe_exchange products.txt ./tests/dud_trader

echo "test 1"
echo "==========================="
./pe_exchange products.txt ./tests/dud_trader > ./tests/dud_trader_output.txt
diff ./tests/dud_trader_expected.txt ./tests/dud_trader_output.txt
echo "==========================="
echo "test 1 finished"

