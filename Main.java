public class Main {
    public static void main(String[] args) {
        Handler textHandler = new TextFileHandler("Text Handler");
        Handler docHandler = new DocFileHandler("Document Handler");
        Handler imageHandler = new ImageFileHandler("Image Handler");

        // Set up Chain of Responsibility
        textHandler.setHandler(docHandler);
        docHandler.setHandler(imageHandler);

        // Test cases
        File file1 = new File("OOP.txt", "text", "C:/OOP");
        File file2 = new File("bore_pen_young.doc", "document", "D:/");
        File file3 = new File("HelloWorld.jpg", "image", "C:/");

        textHandler.process(file1);
        textHandler.process(file2);
        textHandler.process(file3);
    }
}
//การทำงานของโปรแกรม
//TextFileHandler จะตรวจสอบว่าไฟล์เป็น text หรือไม่ ถ้าใช่ก็ประมวลผล มิฉะนั้นจะส่งต่อให้ DocFileHandler
//DocFileHandler จะตรวจสอบว่าไฟล์เป็น document หรือไม่ ถ้าใช่ก็ประมวลผล มิฉะนั้นจะส่งต่อให้ ImageFileHandler
//ImageFileHandler จะตรวจสอบว่าไฟล์เป็น image หรือไม่ ถ้าใช่ก็ประมวลผล ถ้าไม่ตรงกับเงื่อนไขใดเลย จะแสดงข้อความ "File not supported"