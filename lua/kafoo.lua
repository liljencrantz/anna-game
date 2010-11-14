local clock = os.clock
function sleep(n)  -- seconds
  local t0 = clock()
  while clock() - t0 <= n do end
end

while true do
    sleep(1.0);
    print("Inthread");
end
