// http://tsumeguruma.blog46.fc2.com/blog-category-2.html
// http://www.appelsiini.net/2011/simple-usart-with-avr-libc
// https://books.google.co.jp/books?id=oigWAgAAQBAJ&pg=PA102&lpg=PA102&dq=USBS0&source=bl&ots=q9nD2oTFos&sig=InGINp848iUTTIgIL1sHgM0yXWA&hl=ja&sa=X&ved=0ahUKEwjUmuHl_oLMAhWRQpQKHWVgAzcQ6AEIVTAG#v=onepage&q=USBS0&f=false

#define SERVO_MIN 3500 // サーボ最小動作角(-135度) 
#define SERVO_CENTER 7500 // サーボ中心角(0度) 
#define SERVO_MAX 11500 // サーボ最大動作角(+135度) 

float theta = 0.0; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  // USART動作設定 非同期動作 ビット長8bit ストップ1bit パリティEVEN(偶数) 
  UCSR0C = (0<<UMSEL00) | (3<<UCSZ00) | (0<<USBS0) | (1<<UPM01) | (0<<UPM00); 
  delay(1000);
//  ics2_set_pos(0, SERVO_MIN); 
}

void ics2_set_pos(unsigned int id, unsigned int position) { 
  unsigned char cmd, pos_h, pos_l; 
  // CMD コマンドとID 
  cmd = (unsigned char)(128 + id); 
  // POS サーボの設定舵角 
  pos_h = (unsigned char)(position / 128); // 上位7bit 
  pos_l = (unsigned char)(position % 128); // 下位7bit 

  // 1バイトのデータとして送信 
  Serial.write(cmd);
  Serial.write(pos_h);
  Serial.write(pos_l);
//Serial.print(cmd); Serial.print(","); 
//Serial.print(pos_h); Serial.print(",");
//Serial.println(pos_l);
} 

void loop() {
  // put your main code here, to run repeatedly:
  ics2_set_pos(0, SERVO_CENTER + (unsigned int)(1000*sin(theta))); 
  theta += 0.05; 
  delay(500); // 500ms一時停止 
}
