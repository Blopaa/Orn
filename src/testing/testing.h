//
// Created by pablo on 31/08/2025.
//

#ifndef UNTITLED22_TESTING_H
#define UNTITLED22_TESTING_H

// Funciones principales de testing
void runTests(void);

// Tests específicos
void testBasicCases(void);
void testErrorCases(void);
void testCommentCases(void);

void testTokenCount(const char *name, const char* input, int expectedTokens);
void assert(int condition, const char *message);

#endif //UNTITLED22_TESTING_H